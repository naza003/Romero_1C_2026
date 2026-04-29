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
 * | 29/04/2026 | Document creation		                         |
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

/** @def Retardo_tarea1
 * @brief Variable para controlar el retardo de la tarea 1 (lectura de teclas) [ms]
 */
#define Retardo_tarea1 1000

/** @brief Variable para controlar si hay que medir la distancia o no (activar o desactivar la medición). 
 * Además esta variable define el estado inicial del sistema, que es no medir la distancia.
 */
bool medir_distancia = true;

/** @brief Variable para controlar el hold (mantener o no el resultado).
 * Además esta variable define el estado inicial del sistema, que es no mantener el resultado (hold desactivado).
 */
bool hold = false;

/** @brief Variable que guarda la distancia medida en cm
*/
uint16_t distancia = 0;

/** @brief Variable que guarda la ultima distancia medida en cm, para utilizar cuando hold == true, o 
 * en el caso de que  * ocurra la situación medir_distancia == false pero hold == true
 */
uint16_t ultima_distancia = 0;

/*==================[internal data definition]===============================*/
TaskHandle_t MedirEncender_task_handle = NULL;


/*==================[internal functions declaration]=========================*/

/**
 * @brief Función que atiende a la Tecla 1, que activa o desactiva la medición de distancia.
 */
void switch1_interrupcion (void* param){
    medir_distancia =! medir_distancia;   /* Cambia el estado de medir_distancia */
}

/**
 * @brief Función que atiende a la Tecla 2, que cambia el estado del hold (mantener o no el resultado).
 */
void switch2_interrupcion (void* param){
    hold =! hold;                       /* Cambia el estado del hold */
}

/**
 * @brief Función invocada en la interrupción del timer A
 */
//void FuncTimerA(void* param){
//    vTaskNotifyGiveFromISR(MedirEncender_task_handle, pdFALSE);    /* Envía una notificación a la tarea 1 */
//}

/** @fn ActivarLedsSegunDistancia(void)
 * @brief Función que mide la distancia con el sensor de ultrasonido y activa los leds según la distancia medida.
 * @param [in] distancia_ distancia medida por el sensor de ultrasonido en cm.
 */
void ActivarLedsSegunDistancia(uint16_t distancia_){

    /** Se apagan todos los LEDs al inicio para evitar estados residuales de iteraciones anteriores, 
    * y luego se encienden únicamente los correspondientes al rango actual de distancia. */

    LedsOffAll(); //Al iniciar el primer if (<10) no haría nada porque los leds ya están apagados

    if (distancia_<10) {
        LedsOffAll();
    }   

    if (distancia_>=10 && distancia_<20){
        LedOn(LED_1);
    }

    if(distancia_>=20 && distancia_<30){
        LedOn(LED_1);
        LedOn(LED_2);
    }

    if(distancia_>=30){
        LedOn(LED_1);
        LedOn(LED_2);
        LedOn(LED_3);
    }
}

/** @fn static void MedirEncender(void *pvParameter)
 * @brief funcion que decide si medir o no la distancia, dependiendo del estado de medir_distancia. Si mide la 
 * distancia entonces llama a la función que activa los leds.
 * Esta función también decide si se muestra o no la distancia medida en el display LCD, dependiendo del 
 * estado del hold. 
 * @param[in] pvParameter puntero a void que se pasa a la función al crear la tarea, no se utiliza en este caso.
 */
static void MedirEncenderMostrar(void *pvParameter){
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
//    timer_config_t timer_1 = {
//        .timer = TIMER_A, 
//        .period = CONFIG_BLINK_PERIOD_LED_1_US, 
//        .func_p = FuncTimerA, 
//        .param_p = NULL
//    };

//    TimerInit(&timer_1);

    /* Inicialización del conteo de timers */
//    TimerStart(timer_1.timer);   

    /* Creación de tareas */
    xTaskCreate(&MedirEncenderMostrar, "Tarea 1", 512, NULL, 5, &MedirEncender_task_handle);

    /* Interrupciones por switch*/
    SwitchActivInt(SWITCH_1, switch1_interrupcion, NULL);
    SwitchActivInt(SWITCH_2, switch2_interrupcion, NULL);

}
