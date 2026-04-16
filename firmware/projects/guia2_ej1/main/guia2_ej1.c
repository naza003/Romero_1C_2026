/*! @mainpage Guia 2 - Ejercicio 1 - Medidor de distancia por ultrasonido
 *
 * \section genDesc General Description
 *
 * Diseñar el firmware modelando con un diagrama de flujo de manera que cumpla con 
 * las siguientes funcionalidades:
 * Mostrar distancia medida utilizando los leds de la siguiente manera:
 * Si la distancia es menor a 10 cm, apagar todos los LEDs.
 * Si la distancia está entre 10 y 20 cm, encender el LED_1.
 * Si la distancia está entre 20 y 30 cm, encender el LED_2 y LED_1.
 * Si la distancia es mayor a 30 cm, encender el LED_3, LED_2 y LED_1.
 *
 * Mostrar el valor de distancia en cm utilizando el display LCD.
 * Usar TEC1 para activar y detener la medición.
 * Usar TEC2 para mantener el resultado (“HOLD”).
 * Refresco de medición: 1 s
 *
 * Se deberá conectar a la EDU-ESP un sensor de ultrasonido HC-SR04 y una pantalla LCD y utilizando los drivers 
 * provistos por la cátedra implementar la aplicación correspondiente. Se debe subir al repositorio el código. 
 * Se debe incluir en la documentación, realizada con doxygen, el diagrama de flujo. 

 * @note When ussing dedicated connector in ESP-EDU:
 * |   HC_SR04      |   EDU-CIAA	|
 * |:--------------:|:-------------:|
 * | 	Vcc 	    |	5V      	|
 * | 	Echo		| 	GPIO_3		|
 * | 	Trig	 	| 	GPIO_2		|
 * | 	Gnd 	    | 	GND     	| 

 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 15/04/2026 | Document creation		                         |
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
#include "led.h"
#include "switch.h"
#include "lcditse0803.h"
#include "hc_sr04.h"

/*==================[macros and definitions]=================================*/

/** @def Retardo_tarea1
 * @brief Variable para controlar el retardo de la tarea 1 (lectura de teclas) [ms]
 */
#define Retardo_tarea1 1000

/** @def medir_distancia
 * @brief Variable para controlar el retardo de la tarea 2 (lectura de distancia, control de leds y display) [ms]
 */
#define Retardo_tarea2 10

/** @brief Variable para controlar si hay que medir la distancia o no (activar o desactivar la medición)
 */
bool medir_distancia = false;

/** @brief Variable para controlar el hold (mantener o no el resultado)
 */
bool hold = false;

/*==================[internal data definition]===============================*/
TaskHandle_t MedirEncender_task_handle = NULL;
TaskHandle_t LeerTeclas_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

/** @fn ActivarLedsSegunDistancia(void)
 * @brief Función que mide la distancia con el sensor de ultrasonido y activa los leds según la distancia medida.
 * @param [in] distancia_ distancia medida por el sensor de ultrasonido en cm.
 */
void ActivarLedsSegunDistancia(uint16_t distancia_){

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
static void MedirEncender(void *pvParameter){
    while(true){
        
        uint16_t distancia = HcSr04ReadDistanceInCentimeters();

        if(medir_distancia==true){
            ActivarLedsSegunDistancia(distancia);
        }

        if(hold==true){
            LcdItsE0803Write(distancia);
        }
 
        vTaskDelay(Retardo_tarea1 / portTICK_PERIOD_MS);
    }
}


/** @fn static void LeerTeclas(void *pvParameter)
 * @brief funcion que lee el estado de las teclas y dependiendo de cual tecla se presiona, cambia el estado de 
 * los parámetros booleano medir_distancia y hold.
 * @param[in] pvParameter puntero a void que se pasa a la función al crear la tarea, no se utiliza en este caso.
 */
static void LeerTeclas(void *pvParameter){

    while(true){

        int8_t teclas = SwitchesRead();  //leo el estado de las teclas

		switch(teclas){

			case SWITCH_1:
                medir_distancia =! medir_distancia; //si se presiona la tecla 1, se activa o desactiva la medición de distancia
            break;
			
            case SWITCH_2:
                hold =! hold; //si se presiona la tecla 2, se activa o desactiva el hold (mantener o no el resultado)
            break;
        }

        vTaskDelay(Retardo_tarea2 / portTICK_PERIOD_MS); 
    }
}


/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit();
    SwitchesInit();
    LcdItsE0803Init();
    HcSr04Init(GPIO_3, GPIO_2); //GPIO_3 es el echo y GPIO_2 es el trigger

    xTaskCreate(&MedirEncender, "Tarea 1", 512, NULL, 5, &MedirEncender_task_handle); 
    xTaskCreate(&LeerTeclas, "Tarea 2", 512, NULL, 5, &LeerTeclas_task_handle); 
}
