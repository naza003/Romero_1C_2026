/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
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

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Ejercicio 2 Guia 1\n");
	LedsInit();
	SwitchesInit();
	const uint32_t delay_ms = 3000;

	while(true){
		uint8_t teclas = SwitchesRead();  // Defino la variable teclas y LEO EL ESTADO DE LOS SWITCHES
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