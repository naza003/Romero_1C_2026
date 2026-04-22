/*! @mainpage Guia 2 - Ejercicio 2
 *
 * \section genDesc General Description
 *
 * Cree un nuevo proyecto en el que modifique la actividad del punto 1 de manera de utilizar 
 * interrupciones para el control de las teclas y el control de tiempos (Timers)
 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 22/04/2026 | Document creation		                         |
 *
 * @author Nazarena Romero (romeronaza030@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "led.h"
#include "switch.h"
#include "lcditse0803.h"
#include "hc_sr04.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_LED_1_US 1000000  


/*==================[internal data definition]===============================*/
TaskHandle_t MedirEncender_task_handle = NULL;


/*==================[internal functions declaration]=========================*/
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(MedirEncender_task_handle, pdFALSE);    /* Envía una notificación a la tarea 1 */
}


/** @fn static void MedirEncender(void *pvParameter)
 * @brief funcion que decide si medir o no la distancia, dependiendo del estado de medir_distancia. Si mide la 
 * distancia entonces llama a la función que activa los leds.
 * Esta función también decide si se muestra o no la distancia medida en el display LCD, dependiendo del 
 * estado del hold. 
 * @param[in] pvParameter puntero a void que se pasa a la función al crear la tarea, no se utiliza en este caso.
 */
static void MedirEncender(void *pvParameter){
    while(true){
        
        if(medir_distancia==true){

            distancia = HcSr04ReadDistanceInCentimeters();
            ultima_distancia = distancia;      
            ActivarLedsSegunDistancia(distancia);

        }

        if(hold==true){                     // mostrar el último valor, aunque deje de medir
            
            LcdItsE0803Write(ultima_distancia);
        }
 
        vTaskDelay(Retardo_tarea1 / portTICK_PERIOD_MS);
    }
}


/*==================[external functions definition]==========================*/
void app_main(void){

    LedsInit();
    SwitchesInit();
    LcdItsE0803Init();
    HcSr04Init(GPIO_3, GPIO_2); //GPIO_3 es el echo y GPIO_2 es el trigger


    /* Inicialización de timers */
    timer_config_t timer_1 = {
        .timer = TIMER_A, 
        .period = CONFIG_BLINK_PERIOD_LED_1_US, 
        .func_p = FuncTimerA, 
        .param_p = NULL
    };

    TimerInit(&timer_1);

    /* Creación de tareas */
    xTaskCreate(&MedirEncender, "Tarea 1", 512, NULL, 5, &MedirEncender_task_handle);

    
    /* Inicialización del conteo de timers */
    TimerStart(timer_1.timer);

}
