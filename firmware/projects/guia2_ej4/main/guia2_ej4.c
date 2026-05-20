/*! @mainpage Guia 2 - Ejercicio 4
 *
 * @section genDesc General Description
 *
 * Diseñar e implementar una aplicación, basada en el driver analog io mcu.y el driver
 * de transmisión serie uart mcu.h, que digitalice una señal analógica y la transmita 
 * a un graficador de puerto serie de la PC. Se debe tomar la entrada CH1 del conversor AD 
 * y la transmisión se debe realizar por la UART conectada al puerto serie de la PC, 
 * en un formato compatible con un graficador por puerto serie. 
 * Sugerencias:
 * Disparar la conversión AD a través de una interrupción periódica de timer.
 * Utilice una frecuencia de muestreo de 500Hz.
 * Obtener los datos en una variable que le permita almacenar todos los bits del conversor.
 * Transmitir los datos por la UART en formato ASCII a una velocidad de transmisión suficiente para realizar 
 * conversiones a la frecuencia requerida.
 * 
 * Convierta una señal digital de un ECG (provista por la cátedra) en una señal analógica y visualice esta señal 
 * utilizando el osciloscopio que acaba de implementar. Se sugiere utilizar el potenciómetro para conectar la 
 * salida del DAC a la entrada CH1 del AD.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 13/05/2026 | Document creation		                         |
 *
 * @author Nazarena Romero (nazarena.romero@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "timer_mcu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*==================[macros and definitions]=================================*/

/** @def timer_adcUS
 * @brief Variable para controlar el timer de la tarea de lectura del ADC [us]
 */
#define timer_adcUS 2000

/** @def timer_dacUS
 * @brief Variable para controlar el timer de la tarea de lectura del DAC [us]
 */
#define timer_dacUS 20000

/** @def BUFFER_SIZE
 * @brief Variable para controlar el tamaño del buffer de transmisión por UART, 
 * debe ser lo suficientemente grande para almacenar el mensaje a enviar por UART.
 */
#define BUFFER_SIZE 231


/*==================[internal data definition]===============================*/
TaskHandle_t ConvertirAD_task_handle = NULL;
TaskHandle_t ConvertirDA_task_handle = NULL;

static const uint8_t ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
    };

/*==================[internal functions declaration]=========================*/

/** @fn void TimerInterrupcion(void* param)
 * @brief Función invocada en la interrupción por timer, que envía una notificación a la tarea de ADC
 *  y que esta pueda o no convertir la señal analógica y transmitirla por UART.
 */
void TimerInterrupcion1(void* param){
    vTaskNotifyGiveFromISR(ConvertirAD_task_handle, pdFALSE);    /* Envía una notificación a la tarea de ADC para interrumpirla */
}

void TimerInterrupcion2(void* param){
    vTaskNotifyGiveFromISR(ConvertirDA_task_handle, pdFALSE);    /* Envía una notificación a la tarea de DAC para interrumpirla */
}

/** @fn static void ConvertirAD(void *pvParameter)
 * @brief tarea que convierte la señal analógica a digital y la transmite por UART.
 * @param[in] pvParameter puntero a void que se pasa a la función al crear la tarea, no se utiliza en este caso.
 */
static void ConvertirAD(void *pvParameter){

	uint16_t adc_value = 0;  // Variable para almacenar el valor digitalizado del ADC
    char uart_msg[32];	// Buffer para almacenar el mensaje a enviar por UART

    while(true){
        
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
        
        AnalogInputReadSingle(CH1, &adc_value);
        
        snprintf(uart_msg, sizeof(uart_msg), ">brightness:%u\r\n", adc_value);  // Envía el dato en el formato esperado por el graficador serie
                
        UartSendString(UART_PC, uart_msg);   // Envía el mensaje por UART al puerto serie de la PC
        
    }
}

/** @fn static void ConvertirDA(void *pvParameter)
 * @brief tarea que convierte la señal digital del ECG a analógica y la transmite por el DAC.
 * @param[in] pvParameter puntero a void que se pasa a la función al crear la tarea, no se utiliza en este caso.
 */

static void ConvertirDA(void *pvParameter){
    static size_t ecg_indice = 0;  // Índice para recorrer el array de datos del ECG

    while(true){
        
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
        
        AnalogOutputWrite(ecg[ecg_indice]);  // Escribe el valor del ECG en la salida analógica (DAC)
       
        ecg_indice = (ecg_indice + 1) % BUFFER_SIZE;  // Avanza al siguiente valor del ECG
    
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");

	/* Inicalización de los uart */
    serial_config_t configuracion_uart = {
        .port = UART_PC, 
        .baud_rate = 115200, 
        .func_p = NULL, 
        .param_p = NULL
    };

    UartInit(&configuracion_uart);

	/* Inicalización del ADC */
	analog_input_config_t configuracion_adc = {
		.input = CH1, 
		.mode = ADC_SINGLE, 
		.func_p = NULL, 
		.param_p = NULL,
		.sample_frec = 0
	};

	AnalogInputInit(&configuracion_adc);

    /* Inicalización del DAC */
    AnalogOutputInit();

	/* Inicialización de timers */
    timer_config_t timer_1 = {
        .timer = TIMER_A, 
        .period = timer_adcUS, 
        .func_p = TimerInterrupcion1, 
        .param_p = NULL
    };

    TimerInit(&timer_1);

    timer_config_t timer_2 = {
        .timer = TIMER_B, 
        .period = timer_dacUS, 
        .func_p = TimerInterrupcion2, 
        .param_p = NULL
    };

    TimerInit(&timer_2);

	/* Creación de tareas */
    xTaskCreate(&ConvertirAD, "ADC", 2048, NULL, 5, &ConvertirAD_task_handle);
    xTaskCreate(&ConvertirDA, "DAC", 2048, NULL, 5, &ConvertirDA_task_handle);

	TimerStart(timer_1.timer);  // Inicia el timer para que comience a generar interrupciones periódicas
	TimerStart(timer_2.timer);  // Inicia el timer para que comience a generar interrupciones periódicas
}
/*==================[end of file]============================================*/