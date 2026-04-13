/*! @mainpage GUIA 1 - EJERCICIO 2
 *
 * @section genDesc General Description
 *
 * Modifique la aplicación 1_blinking_switch de manera de hacer titilar los leds 1 y 2 al 
 * mantener presionada las teclas 1 y 2 correspondientemente. También se debe poder hacer titilar el
 * led 3 al presionar simultáneamente las teclas 1 y 2
 *
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_12	 	| 	GPIO_11		|
 * | 	PIN_11	 	| 	GPIO_10 	|
 * | 	PIN_5	 	| 	GPIO_5		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/03/2026 | Document creation		                         |
 *
 * @author Romero Nazarena (romeronaza030@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <led.h>
#include <switch.h>
/*==================[macros and definitions]=================================*/

/** @def delay_ms
 * @brief Tiempo de retardo en milisegundos
 */
#define delay_ms 3000

/** @brief Variable que guarda el estado del switch que se está leyendo */
uint8_t teclas = 0;

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){

	printf("Ejercicio 2 Guia 1\n");
	LedsInit();
	SwitchesInit();

	while(true){
		teclas = SwitchesRead();  
		switch(teclas){
			case SWITCH_1:
				LedOn(LED_1);
				vTaskDelay(pdMS_TO_TICKS(delay_ms)); // MANTENGO EL LED PRENDIDO POR 3 SEGUNDOS (3000ms) Y LUEGO LO APAGO
				LedOff(LED_1);		
			break;

			case SWITCH_2:
				LedOn(LED_2);
				vTaskDelay(pdMS_TO_TICKS(delay_ms));
				LedOff(LED_2);
			break;
			
			case SWITCH_1 | SWITCH_2: // SI SE PRESIONAN AMBOS SWITCHES, INVIERTO EL ESTADO DEL LED_3    
				LedOn(LED_3);
				vTaskDelay(pdMS_TO_TICKS(delay_ms));
				LedOff(LED_3);
			break;
			default:
				LedsOffAll();	// APAGO TODOS LOS LEDS
		}

	}

}
/*==================[end of file]============================================*/