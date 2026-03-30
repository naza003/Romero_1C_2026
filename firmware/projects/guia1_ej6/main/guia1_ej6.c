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
#include <freertos/FreeRTOS.h> // Incluyo la libreria de FreeRTOS para usar la funcion vTaskDelay
#include <freertos/task.h> // Incluyo la libreria de FreeRTOS para usar la funcion vTaskDelay

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

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

uint8_t ConversionYMultiplexado (uint8_t numerodecimal_, uint8_t digitos_, uint8_t *bcd_number, gpio_t *gpio_map, gpio_t *gpio_pulso) //dos vectores de estructuras
{
	//**************Lo primero es convertir el decimal a bcd**********************
	if (bcd_number == NULL) {
		return -1;
	} // el puntero no puede ser nulo

	for (int8_t i = digitos_-1; i>=0; i--){	//se lee de derecha a izquierda y cuenta desde el 0
		bcd_number[i] = numerodecimal_ % 10; // guardo el ultimo digito del numero en el array
		numerodecimal_ /= 10; // elimino el ultimo digito del numero
	}

	if (numerodecimal_ != 0){
		return -1; // el numero tiene más digitos que el tamaño del array
	}
	
	//*************Prender o apagar el gpio dependiendo si se trata de un 1 o 0 // MULTIPLEXADO****************
	for(int8_t j=0; j<digitos_; j++){

		// Envio los digitos al gpio 20 a 23
		GPIO_onoff(bcd_number[j], gpio_map);

		//Enciendo el display correspondiente
		GPIOOn(gpio_pulso[j]);

		//Espero un tiempo para que el display muestre el numero
		vTaskDelay(pdMS_TO_TICKS(5)); // 5ms es un tiempo razonable para que el display muestre el numero

		//Apago el display para mostrar el siguiente numero
		GPIOOff(gpio_pulso[j]);
	}

	return 0; // la funcion se ejecuto correctamente
}



/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Guia1_ej6\n");

	//Inicializo los pins que voy a utilizar
	GPIOInit(GPIO_19, GPIO_OUTPUT);
	GPIOInit(GPIO_18, GPIO_OUTPUT);
	GPIOInit(GPIO_9, GPIO_OUTPUT);
	GPIOInit(GPIO_20, GPIO_OUTPUT);
	GPIOInit(GPIO_21, GPIO_OUTPUT);
	GPIOInit(GPIO_22, GPIO_OUTPUT);
	GPIOInit(GPIO_23, GPIO_OUTPUT);

	uint8_t numerodecimal = 123;
	uint8_t digitos = 3;
	uint8_t bcd_number [digitos]; 

	gpio_t gpio_map[4] = {GPIO_20, GPIO_21, GPIO_22, GPIO_23}; 
	gpio_t gpio_pulso[3] = {GPIO_19, GPIO_18, GPIO_9};

	int8_t tiempo = 0;
	while (tiempo < 5000){
		ConversionYMultiplexado(numerodecimal, digitos, bcd_number, gpio_map, gpio_pulso);
		tiempo++;
	}
}
/*==================[end of file]============================================*/