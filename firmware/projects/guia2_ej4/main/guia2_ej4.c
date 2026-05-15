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

/*==================[internal data definition]===============================*/
TaskHandle_t ConvertirAD_task_handle = NULL;

/**
 * @brief Función invocada en la interrupción por timer, que envía una notificación a la tarea de ADC
 *  y que esta pueda o no convertir la señal analógica y transmitirla por UART.
 */
void TimerInterrupcion(void* param){
    vTaskNotifyGiveFromISR(ConvertirAD_task_handle, pdFALSE);    /* Envía una notificación a la tarea de ADC para interrumpirla */
}

/*==================[internal functions declaration]=========================*/

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

	/* Inicialización de timers */
    timer_config_t timer_1 = {
        .timer = TIMER_A, 
        .period = timer_adcUS, 
        .func_p = TimerInterrupcion, 
        .param_p = NULL
    };

    TimerInit(&timer_1);

	/* Creación de tareas */
    xTaskCreate(&ConvertirAD, "ADC", 2048, NULL, 5, &ConvertirAD_task_handle);

	TimerStart(timer_1.timer);  // Inicia el timer para que comience a generar interrupciones periódicas

}
/*==================[end of file]============================================*/