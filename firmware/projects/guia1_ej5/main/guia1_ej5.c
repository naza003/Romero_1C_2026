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
 * | 25/03/2026 | Document creation		                         |
 *
 * @author Nazarena Romero (romeronaza030@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/


typedef struct 
{
	gpio_t pin; 	// numero de pin
	io_t dir;		// input=0 o output=1
} gpioConf_t;


/*==================[internal functions declaration]=========================*/

void GPIO_onoff (uint8_t bcd_number_ ,gpio_t *gpio_map){
	
	for(int8_t i=0; i<4; i++){

		uint8_t bit= bcd_number_>>i & 1; 

		if (bit==1){
			GPIOOn(gpio_map[i]);
		}
		if (bit==0){
			GPIOOff(gpio_map[i]);
		}
	}

}

/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Guia 1 - Ejercicio 5\n");

	//inicializo los pins del gpio que voy a utilizar
	GPIOInit(GPIO_20, GPIO_OUTPUT);
	GPIOInit(GPIO_21, GPIO_OUTPUT);
	GPIOInit(GPIO_22, GPIO_OUTPUT);
	GPIOInit(GPIO_23, GPIO_OUTPUT);

	uint8_t bcd_number = 0b1010; 
	gpio_t gpio_map[4] = {GPIO_20, GPIO_21, GPIO_22, GPIO_23};
	GPIO_onoff(bcd_number, gpio_map);

}
/*==================[end of file]============================================*/