/*! @mainpage Proyecto Integrador
 *
 * @section genDesc General Description
 *
 * BIOFEEDBACK DE PRESIÓN DE ESPIRACIÓN PARA REHABILITACIÓN RESPIRATORIA
 * El proyecto consiste en un dispositivo de medición de presión de soplo para pacientes con 
 * enfermedades respiratorias que requieren rehabilitación. 
 * Utiliza un sensor de presión MP3V5050 para medir la presión del aire exhalado por el paciente, y utiliza
 * un NeoPixel de 12 LEDs para proporcionar retroalimentación visual al paciente. 
 * Tiene dos modos de medición: expiración fuerte y expiración mantenida.
 * En el modo de expiración fuerte, el NeoPixel muestra la intensidad del soplo mediante un mapeo de colores
 * y cantidad de LEDs encendidos. Este se activa al presionar la Tecla 1.
 * En el modo de expiración mantenida, el NeoPixel muestra el progreso del soplo hacia los 4 segundos 
 * requeridos para completar la prueba. Este se activa al presionar la Tecla 2.
 * El dispositivo también envía los resultados de la medición a través de Bluetooth o UART 
 * para su análisis posterior.
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
#include <led.h>

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
 * @brief Variable booleana que indica el tipo de expiración a medir.  
*/
bool expiracion_fuerte = false;

/**
 * @brief Variable booleana que indica el tipo de expiración a medir.
 */
bool expiracion_mantenida = false;

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

/**
 * @brief Controla cuánto tiempo se muestran el rojo/verde
 */
uint32_t tiempo_espera_visual_ms = 0;    

/**
 * @brief Buffer para mensaje de resultados del modo fuerte
 */
char msg1[64]; 

/**
 * @brief Buffer para mensaje de resultados del modo mantenido
 */
char msg2[64]; 

/*==================[internal data definition]===============================*/
TaskHandle_t Sensado_task_handle = NULL;
TaskHandle_t ControlBle_task_handle = NULL;

/*==================[internal functions declaration]================================*/

/**
 * @brief Función que atiende a la Tecla 1, que activa o desactiva la medición de presión 
 * y el modo de expiración fuerte.
 */
void Tecla1_interrupcion (void* param){
    medir_presion = !medir_presion;   /* Cambia el estado de medir_presion */
    expiracion_fuerte = medir_presion;
    expiracion_mantenida = false; /* Solo puede estar activo un modo a la vez */ 

    // Reseteo variables globales de terapia por seguridad
    soplando = false;
    tiempo_espiracion_ms = 0;
    volumen_relativo_acumulado = 0.0f;
    presion_maxima = 0.0f;
}

/**
 * @brief Función que atiende a la Tecla 2, que activa o desactiva la medición de presión 
 * y el modo de expiración mantenida.
 */
void Tecla2_interrupcion (void* param){
    medir_presion = !medir_presion;   /* Cambia el estado de medir_presion */
    expiracion_mantenida = medir_presion;
    expiracion_fuerte = false; /* Solo puede estar activo un modo a la vez */
    
    tiempo_espiracion_ms = 0; // Reinicio el reloj por las dudas
    soplando = false; 
        
    // Se ACTIVA el modo, todos los leds se prenden 
    if (expiracion_mantenida == true) {
        for(int i = 0; i < 12; i++){
            pixel_data[i] = NEOPIXEL_COLOR_WHITE; /* Le aviso que está listo para comenzar */
        }
        NeoPixelSetArray(pixel_data); 
    } else {
        // se DESACTIVA el modo, apago todos los leds
        for(int i = 0; i < 12; i++){
            pixel_data[i] = NEOPIXEL_COLOR_BLACK; 
        }
        NeoPixelSetArray(pixel_data); 
    }
    
}

/**
 * @brief Función callback que se ejecuta cuando llega un dato por Bluetooth
 */
void RecibirDatoBluetooth(uint8_t *data, uint8_t length) {
    // Tomamos el primer caracter del dato recibido
    char comando = (char)data[0];

    if (comando == 'f') {
        // --- EQUIVALENTE A PRESIONAR SWITCH 1 (Modo Fuerte) ---
        medir_presion = !medir_presion;   
        expiracion_fuerte = medir_presion;
        expiracion_mantenida = false; 

        soplando = false;
        tiempo_espiracion_ms = 0;
        volumen_relativo_acumulado = 0.0f;
        presion_maxima = 0.0f;
    } 
    else if (comando == 'm') {
        // --- EQUIVALENTE A PRESIONAR SWITCH 2 (Modo Mantenido) ---
        medir_presion = !medir_presion;   
        expiracion_mantenida = medir_presion;
        expiracion_fuerte = false; 
        
        tiempo_espiracion_ms = 0; 
        soplando = false;
            
        if (expiracion_mantenida == true) {
            for(int i = 0; i < 12; i++){
                pixel_data[i] = NEOPIXEL_COLOR_WHITE; 
            }
            NeoPixelSetArray(pixel_data); 
        } else {
            for(int i = 0; i < 12; i++){
                pixel_data[i] = NEOPIXEL_COLOR_BLACK; 
            }
            NeoPixelSetArray(pixel_data); 
        }
    }
}

/** @fn static void ExpiracionFuerte(float presion_neta_kPa_1)
 * @brief Función que mapea el valor de presión medida a un color específico para el NeoPixel.
 * @param[in] presion_neta_kPa_1 valor de presión neta medido en kPa, que se mapea a un color según los umbrales definidos.
 */

static void ExpiracionFuerte(float presion_neta_kPa_1){
    
    /* 1. Calculo cuántos leds prender utilizando regla de tres (mapeo lineal) */
    /* Rango de presión:  0.84kPa (0 LEDs) a 2.7kPa (3.54-0.84) (12 LEDs) */
    
    int leds_a_encender = (int)((presion_neta_kPa_1 / 2.69f) * 12.0f);
    
    /* Contol de límites (si el paciente sopla con demás fuerza la regla de tres puede dar > 12LEDs) */
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
           }    else if (i<12) {
                pixel_data[i] = NEOPIXEL_COLOR_VIOLET; /* LEDs 10, 11, 12 --> VIOLETAS */
           } 

        } else {
                pixel_data[i] = NEOPIXEL_COLOR_BLACK; /* LEDs restantes --> APAGADOS */
           }
        
    }

    NeoPixelSetArray(pixel_data); 

    /* 3. Análisis del soplo */
    if (presion_neta_kPa_1 > 0.0f) {
        // EL PACIENTE ESTÁ SOPLANDO
        soplando = true;
                
        // Cronómetro: sumamos los 50ms de este ciclo
        tiempo_espiracion_ms += periodo_muestreo; 
                
        // Integración (Área bajo la curva): sumamos Flujo * Tiempo(en segundos)
        volumen_relativo_acumulado += (presion_neta_kPa_1 * (periodo_muestreo / 1000.0f));

        // Récord de presión: ¿es mayor al máximo de este soplido?
        if (presion_neta_kPa_1 > presion_maxima) {
            presion_maxima = presion_neta_kPa_1;
        }

    } else if (soplando == true && presion_neta_kPa_1 == 0.0f) {
        // EL PACIENTE ACABA DE TERMINAR EL SOPLIDO (Flanco de bajada)
        float tiempo_soplado_segundos = tiempo_espiracion_ms / 1000.0f;

        /* 4. MOSTRAMOS LOS RESULTADOS */
        snprintf(msg1, sizeof(msg1), "*KMax:%.2f kPa | Dur:%.1f s | VolumenRelatvo:%.2f\n*", 
                presion_maxima, tiempo_soplado_segundos, volumen_relativo_acumulado);

        if (BleStatus() == BLE_CONNECTED) {
            BleSendString(msg1);
        } 

        // RESETEAMOS TODO para el siguiente soplido
        soplando = false;
        tiempo_espiracion_ms = 0;
        volumen_relativo_acumulado = 0.0f;
        presion_maxima = 0.0f;
    }    
}

/** @fn static void ExpiracionMantenida(float presion_neta_kPa_2)
 *  @brief Funcion para controlar el NeoPixel en base a la presión medida durante una expiración mantenida. 
 * Si el paciente sopla por más de 4 segundos, se prenden todos los LEDs de color verde y 
 * se envía un mensaje de éxito. Si corta antes de los 4 segundos, se prenden de rojo y 
 * se envía un mensaje de fallo indicando la duración del soplo.
 *  @param[float] presion_neta_kPa_2 presión neta medida durante la expiración mantenida.
 */
static void ExpiracionMantenida(float presion_neta_kPa_2){

    bool enviar_mensaje = false; 

    // Si el temporizador visual está corriendo, mantenemos el color final y descontamos tiempo
    if (tiempo_espera_visual_ms > 0) {
        if (tiempo_espera_visual_ms >= periodo_muestreo) {
            tiempo_espera_visual_ms -= periodo_muestreo;
        } else {
            tiempo_espera_visual_ms = 0;
        }
        return; // Salimos de la función para mantener el color en el NeoPixel sin recalcular
    }

    /* El paciente está espirando?*/
    if (presion_neta_kPa_2 > 0.0f) {
        soplando = true;
    } else soplando = false;

    /* Controlo el neopixel*/
    if(soplando == true && tiempo_espiracion_ms < 2000){
        /* Estado: ARRANCA LA CUENTA REGRESIVA */
        tiempo_espiracion_ms += periodo_muestreo; /* Cronómetro: le sumo los 50ms de este ciclo */

        /*Calculo cuantos leds apagar por segundo (3 cada 1000ms)*/
        int leds_a_apagar = (tiempo_espiracion_ms *12) / 2000;
        int leds_encendidos = 12 - leds_a_apagar;

        for (int i = 0; i < 12; i++){
            if (i < leds_encendidos) {
                pixel_data[i] = NEOPIXEL_COLOR_ROSE; /* Le aviso que está sensando el soplo */
            } else {
                pixel_data[i] = NEOPIXEL_COLOR_BLACK; /* Se van apagando */
            }
        }
        NeoPixelSetArray(pixel_data);
    
    } else if (soplando == true && tiempo_espiracion_ms >= 2000) {   
        /* Estado: TIEMPO COMPLETADO. El paciente sigue espirando*/
        /*Se ponen en verde*/
        for (int i = 0; i < 12; i++) {
            pixel_data[i] = NEOPIXEL_COLOR_GREEN;
        }
        NeoPixelSetArray(pixel_data);

    } else if (soplando == false && tiempo_espiracion_ms >= 2000) { 
        /* Estado: No espira porque TERMINÓ el soplo exitosamente*/
        /* Escribo mensaje */
        snprintf(msg2, sizeof(msg2), "*MEXITO!\n*");
        enviar_mensaje = true;

        // En lugar de vTaskDelay, usamos el temporizador pasivo
        tiempo_espera_visual_ms = 1500; 
        tiempo_espiracion_ms = 0;
        soplando = false;

    } else if (soplando == false && tiempo_espiracion_ms > 0 && tiempo_espiracion_ms < 2000) { /* No espira porque CORTÓ ANTES el aire*/
        /* Escribo mensaje */
        snprintf(msg2, sizeof(msg2), "*MFALLO! duracion: %.1fs\n*", tiempo_espiracion_ms / 1000.0f);
        enviar_mensaje = true; 

        /*Se ponen en rojo*/
        for (int i = 0; i < 12; i++) {
            pixel_data[i] = NEOPIXEL_COLOR_RED;
        }
        NeoPixelSetArray(pixel_data);

        // En lugar de vTaskDelay, usamos el temporizador pasivo
        tiempo_espera_visual_ms = 1500; 
        tiempo_espiracion_ms = 0;
        soplando = false;
            
    } else { 
        /* Estado: EN ESPERA. Aún no comenzó el soplo y el tiempo = 0*/
        for (int i = 0; i < 12; i++) {
            pixel_data[i] = NEOPIXEL_COLOR_WHITE; /* Para indicar que está listo para comenzar */
        }
        NeoPixelSetArray(pixel_data);
    }

    /* Envío del mensaje */
    if (enviar_mensaje==true){
        if(BleStatus() == BLE_CONNECTED) {
            BleSendString(msg2);
        } 
    }

}
   
/** @fn static void Sensado(void *pvParameters)
 *  @brief Tarea que se encarga de leer la presión medida por el MP3V5050 y llama a la funcion que controla
 *  el NeoPixel en base a la presion medida.
 *  @param[in] pvParameters puntero a void que se pasa a la función al crear la tarea, no se utiliza en este caso.
 */

static void Sensado(void *pvParameters){	

    while (true) { 

        if (medir_presion==true) {
            
            float presion_medida = MP3v5050ReadPressure_kPa();

            /* 1. Resto la linea base (taramos). Sin soplar sensa 0.86kPa*/
            float presion_neta = presion_medida - 0.86f; //  restamos el reposo
            if (presion_neta < 0.15f) {
                presion_neta = 0.0f; // Evitamos valores negativos si el sensor fluctúa a 0.83
            }

            /* 2. Control del NeoPixel según el tipo de expiración */
            if (expiracion_fuerte == true){
                ExpiracionFuerte(presion_neta);
                //Imprimir(NULL);
            }
            else if (expiracion_mantenida == true){
                ExpiracionMantenida(presion_neta);
            }

        } else {
            // Limpiamos el arreglo a mano
            for(int i = 0; i < 12; i++){
                pixel_data[i] = NEOPIXEL_COLOR_BLACK; /* Apagamos todos los LEDs */
            }
            NeoPixelSetArray(pixel_data);
        }

        vTaskDelay(periodo_muestreo / portTICK_PERIOD_MS);
    }
}

/** @fn static void ControlBle (void *pvParameters)
 * @brief Funcion que controla si el bluetooth de la placa está conectado al del celular
 * @param[in] pvParameters puntero a void que se pasa a la función al crear la tarea, no se utiliza en este caso.
 */
static void ControlBle (void *pvParameters){
    
    while(1){
        vTaskDelay(500 / portTICK_PERIOD_MS);
        if(BleStatus() == BLE_CONNECTED) {
            LedOff(LED_2);
            LedToggle(LED_1);
        } else {
            LedOff(LED_1);
            LedToggle(LED_2);
        }
        
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){

	MP3v5050Init();
	SwitchesInit();
    LedsInit();
	NeoPixelInit(GPIO_23, 12, pixel_data);

    NeoPixelAllOff(); /* Aseguramos que los LEDs estén apagados al iniciar el programa */

    /* Interrupciones por switch*/
    SwitchActivInt(SWITCH_1, Tecla1_interrupcion, NULL);
    SwitchActivInt(SWITCH_2, Tecla2_interrupcion, NULL); 

    /* Inicializacion de bluetooth*/
    ble_config_t ble_config = {
        .device_name = "ESP_EDU_Naza",
        .func_p = RecibirDatoBluetooth
	};

	BleInit(&ble_config);

    /* Inicalización de los uart */
    serial_config_t configuracion_uart = {
        .port = UART_PC, 
        .baud_rate = 115200, 
        .func_p = NULL, 
        .param_p = NULL
    };

    UartInit(&configuracion_uart);

	xTaskCreate(&Sensado, "Sensado", 2048, NULL, 5, &Sensado_task_handle);
    xTaskCreate(&ControlBle, "Control del bluetooth", 2048, NULL, 5, &ControlBle_task_handle);
    
}
/*==================[end of file]============================================*/