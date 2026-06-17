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
#include <ble_mcu.h>

/*==================[macros and definitions]=================================*/

/** 
 * @brief Periodo de muestreo para la tarea de sensado, definido en ms.
 * Se define como 1000 ms para obtener una buena resolución en la medición sin saturar el sistema.
*/
#define periodo_muestreo 50 //ms 

/**
 * @brief Variable booleana que cambia de estado según se presione o no la tecla. 
 * Controla el inicio o no de la medición de presión.  
*/
bool medir_presion = false;

/**
 * @brief Guardo 12 lugares en la memoria RAM para los LEDs del NeoPixel
 */
static neopixel_color_t pixel_data[12];

/**
 * @brief Variable para almacenar la presión máxima medida durante la sesión de soplo 

 */
float presion_maxima = 0.0f; 

/**
 * @brief Variable para almacenar el tiempo de espiración en milisegundos
 */
uint32_t tiempo_espiracion_ms = 0; 

/**
 * @brief Variable para acumular el volumen relativo durante la sesión de soplo
 */
float volumen_relativo_acumulado = 0.0f; 

/**
 * @brief Variable booleana para indicar si el paciente está soplando o no
 */
bool soplando = false; 

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

static void ControlNeoPixelProgresivo(float presion_neta_kPa){

    /* 1. Calculo cuántos leds prender utilizando regla de tres (mapeo lineal) */
    /* Rango de presión:  0.84kPa (0 LEDs) a 2.7kPa (3.54-0.84) (12 LEDs) */
    
    int leds_a_encender = (int)((presion_neta_kPa / 2.69f) * 12.0f);
    
    /* Contol de límites (si el paciente sopla fuerte la regla de tres puede dar > 12LEDs) */
    if (leds_a_encender < 0) leds_a_encender = 0;
    if (leds_a_encender > 12) leds_a_encender = 12;

    /* 2. Bucle para recorrer los 12 LEDs */
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
    char msg[64];	

    while (true) { 

        if (medir_presion==true) {
            
            float presion_medida = MP3v5050ReadPressure_kPa();

            /* 1. Resto la linea base (taramos). Sin soplar sensa 0.86kPa*/
            float presion_neta = presion_medida - 0.86f; //  restamos el reposo
            if (presion_neta < 0.5f) {
                presion_neta = 0.0f; // Evitamos valores negativos si el sensor fluctúa a 0.83
            }

            /* 2. Control del NeoPixel según la presión neta */
            ControlNeoPixelProgresivo(presion_neta);

            /* 3. Análisis del soplo */
            if (presion_neta > 0.0f) {
                // EL PACIENTE ESTÁ SOPLANDO
                soplando = true;
                
                // Cronómetro: sumamos los 50ms de este ciclo
                tiempo_espiracion_ms += periodo_muestreo; 
                
                // Integración (Área bajo la curva): sumamos Flujo * Tiempo(en segundos)
                volumen_relativo_acumulado += (presion_neta * (periodo_muestreo / 1000.0f));

                // Récord de presión: ¿es mayor al máximo de este soplido?
                if (presion_neta > presion_maxima) {
                    presion_maxima = presion_neta;
                }

            } else if (soplando == true && presion_neta == 0.0f) {
                // EL PACIENTE ACABA DE TERMINAR EL SOPLIDO (Flanco de bajada)
                float tiempo_soplado_segundos = tiempo_espiracion_ms / 1000.0f;

                /* 4. MOSTRAMOS LOS RESULTADOS */
                snprintf(msg, sizeof(msg), "*FIN! Max:%.2f kPa | Dur:%.1f s | Vol:%.2f\r\n*", 
                        presion_maxima, tiempo_soplado_segundos, volumen_relativo_acumulado); 

                if (BleStatus() == BLE_CONNECTED) {
                    BleSendString(msg);
                } else {    
                    UartSendString(UART_PC, msg);   // por si no conecta ble
                }

                // RESETEAMOS TODO para el siguiente soplido
                soplando = false;
                tiempo_espiracion_ms = 0;
                volumen_relativo_acumulado = 0.0f;
                presion_maxima = 0.0f;
            }    

//            snprintf(uart_msg, sizeof(uart_msg), ">presión: %.2f\r\n", presion_medida);  // Envía el dato en el formato esperado por el graficador serie
//            UartSendString(UART_PC, uart_msg);   // Envía el mensaje por UART al puerto serie de la PC

        } else {
            // Limpiamos el arreglo a mano para asegurarnos de que limpio
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
    
    ble_config_t ble_config = {
    .device_name = "ESP_EDU_Naza",
    .func_p = NULL
	};

	BleInit(&ble_config);

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