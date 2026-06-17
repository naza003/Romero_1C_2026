/*! @mainpage Proyecto Integrador
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	| Dispositive |
 * |:--------------:|:--------------|-------------|
 * | 	PIN_X	 	| 	GPIO_23		|  NeoPixel   |
 * | 	CH1		 	| 	GPIO_1		|  MP3V5050   |
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 24/05/2026 | Document creation		                         |
 *
 * @author Nazarena Romero (romeronaza030@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <analog_io_mcu.h>
#include <uart_mcu.h>
#include <timer_mcu.h>
#include <neopixel_stripe.h>
#include <mp3v5050.h>
#include <switch.h>

/*==================[macros and definitions]=================================*/

/** 
 * @brief Periodo de muestreo para la tarea de sensado, definido en ms.
 * Se define como 1000 ms para obtener una buena resolución en la medición sin saturar el sistema.
*/
#define periodo_muestreo 50 //ms 

//umbrales seguros para el dispositivo
#define umbral_max_kPa 100
#define umbral_min_kPa 20

//definidos segun lo sensado en el paciente o segun lo definido por bibliografía
#define umbral_1_kPa 0 //reposo + 10-20% para medir inicio de espiración 
#define umbral_2_kPa 50 // reposo + 50-70%
#define umbral_3_kPa 80 // reposo + 80-90%
#define umbral_4_kPa 100 // reposo + 100% para evitar esfuerzo excesivo

/**
 * @brief Variable booleana que cambia de estado según se presione o no la tecla. 
 * Controla el inicio o no de la medición de presión.  
*/
bool medir_presion = false;

/**
 * @brief Guardo 12 lugares en la memoria RAM para los LEDs del NeoPixel
 */
static neopixel_color_t pixel_data[12];


/*==================[internal data definition]===============================*/
TaskHandle_t Sensado_task_handle = NULL;

/*==================[internal functions declaration]================================*/

/**
 * @brief Función que atiende a la Tecla 1, que activa o desactiva la medición de presión.
 */
void Tecla1_interrupcion (void* param){
    medir_presion = !medir_presion;   /* Cambia el estado de medir_presion */
}

/** @fn static void ControlNeoPixelProgresivo(float presion_kPa)
 * @brief Función que mapea el valor de presión medida a un color específico para el NeoPixel.
 * @param[in] presion_kPa valor de presión medido en kPa, que se mapea a un color según los umbrales definidos.
 */

static void ControlNeoPixelProgresivo(float presion_kPa){

    /* 1. Resto la linea base (taramos). Sin soplar sensa 0.86kPa*/
    float presion_neta = presion_kPa - 0.86f; //  restamos el reposo
    if (presion_neta < 0.05f) {
        presion_neta = 0; // Evitamos valores negativos si el sensor fluctúa a 0.83
    }

    /* 2. Calculo cuántos leds prender utilizando regla de tres (mapeo lineal) */
    /* Rango de presión:  0.84kPa (0 LEDs) a 2.7kPa (3.54-0.84) (12 LEDs) */
    
    int leds_a_encender = (int)((presion_neta / 2.69f) * 12.0f);
    
    /* Contol de límites (si el paciente sopla fuerte la regla de tres puede dar > 12LEDs) */
    if (leds_a_encender < 0) leds_a_encender = 0;
    if (leds_a_encender > 12) leds_a_encender = 12;

    /* 3. Bucle para recorrer los 12 LEDs */
    for (int i = 0; i < 12; i++) {

        if (i < leds_a_encender) {

            /* Indico el color del LED encendido según su posición */
            if (i<3) {
                pixel_data[i] = NEOPIXEL_COLOR_RED;  /* LEDs 1, 2 ,3 --> ROJOS */
           } else if (i<6) {
                pixel_data[i] = NEOPIXEL_COLOR_YELLOW; /* LEDs 4, 5, 6 --> AMARILLOS */
           } else if (i<9) {
                pixel_data[i] = NEOPIXEL_COLOR_BLUE; /* LEDs 7, 8, 9 --> AZULES */
           } else if (i<12) {
                pixel_data[i] = NEOPIXEL_COLOR_VIOLET; /* LEDs 10, 11, 12 --> VIOLETAS */
           } 
           
        } else {
                pixel_data[i] = NEOPIXEL_COLOR_BLACK; /* LEDs restantes --> APAGADOS */
           }
        
    }

    /* 3. Envío la orden al NeoPixel */
    NeoPixelSetArray(pixel_data); 

}

/** @fn static void Sensado(void *pvParameters)
 *  @brief Tarea que se encarga de leer la presión medida por el MP3V5050 y llama a la funcion que controla
 *  el NeoPixel en base a la presion medida.
 *  @param[in] pvParameters puntero a void que se pasa a la función al crear la tarea, no se utiliza en este caso.
 */

static void Sensado(void *pvParameters){
     
    /* Buffer para almacenar el mensaje a enviar por UART */
    char uart_msg[32];	

    while (true) { 

        if (medir_presion==true) {
            float presion_medida = MP3v5050ReadPressure_kPa();
            ControlNeoPixelProgresivo(presion_medida);

            snprintf(uart_msg, sizeof(uart_msg), ">presión: %.2f\r\n", presion_medida);  // Envía el dato en el formato esperado por el graficador serie
            UartSendString(UART_PC, uart_msg);   // Envía el mensaje por UART al puerto serie de la PC

        } else {
            // Limpiamos el arreglo a mano para asegurarnos de que quede en negro
            for(int i = 0; i < 12; i++){
                pixel_data[i] = NEOPIXEL_COLOR_BLACK; /* Apagamos todos los LEDs */
            }
            NeoPixelSetArray(pixel_data);
        }

    /* delay */
        vTaskDelay(periodo_muestreo / portTICK_PERIOD_MS);
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");

	MP3v5050Init();
	SwitchesInit();
	NeoPixelInit(GPIO_23, 12, pixel_data);

    //PARA PROBAR LA PROXIMA PARA QUE LOS LEDS SE APAGUEN PORQUE SINO QUEDABAN PRENDIDOS AUNQUE NO SE ESTÉ SOPLANDO.
    NeoPixelAllOff(); /* Aseguramos que los LEDs estén apagados al iniciar el programa */

    /* Interrupciones por switch*/
    SwitchActivInt(SWITCH_1, Tecla1_interrupcion, NULL);

    /* Inicalización de los uart */
    serial_config_t configuracion_uart = {
        .port = UART_PC, 
        .baud_rate = 115200, 
        .func_p = NULL, 
        .param_p = NULL
    };

    UartInit(&configuracion_uart);

	xTaskCreate(&Sensado, "Sensado", 2048, NULL, 5, &Sensado_task_handle);
    
}
/*==================[end of file]============================================*/