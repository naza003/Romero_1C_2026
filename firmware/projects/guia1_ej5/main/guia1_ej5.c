/*! @mainpage GUIA 1 - EJERCICIO 5
 *
 * @section genDesc General Description
 *
 * Escribir una función que reciba como parámetro un dígito BCD
 * y un vector de estructuras del tipo gpioConf_t. Incluya el archivo de cabecera gpio_mcu.h
 * Defina un vector que mapee los bits de la siguiente manera:
 * b0 -> GPIO_20
 * b1 ->  GPIO_21
 * b2 -> GPIO_22
 * b3 -> GPIO_23
 * La función deberá cambiar el estado de cada GPIO, a ‘0’ o a ‘1’, 
 * según el estado del bit correspondiente en el BCD ingresado. Ejemplo: b0 se encuentra en ‘1’,
 * el estado de GPIO_20 debe setearse. 
 * 
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_18	 	| 	GPIO_20		|
 * | 	PIN_19	 	| 	GPIO_21		|
 * | 	PIN_20	 	| 	GPIO_22		|
 * | 	PIN_21	 	| 	GPIO_23		|
 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 25/03/2026 | Document creation		                         |
 * | 13/04/2026 | Document update		                         |
 *
 * @author Nazarena Romero (romeronaza030@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/

/** @brief Estructura para almacenar la información de los GPIOs 
*/
typedef struct 
{
	gpio_t pin; 	/**<numero de pin GPIO*/
	io_t dir;		/**< direccion: input=0 o output=1 */
} gpioConf_t;


/*==================[internal functions declaration]=========================*/

/** @fn void GPIO_onoff (uint8_t bcd_number_, gpioConf_t *gpio_map)
 * @brief Controla el estado de los GPIOs según el valor del dígito BCD recibido. 
 * @param[in] bcd_number_ Dígito BCD (0-9) que determina el estado de los GPIOs.
 * @param[in] gpio_map Vector de estructuras que mapea los bits a los GPIOs configurados.
 */

void GPIO_onoff (uint8_t bcd_number_ ,gpioConf_t *gpio_map){
	
	for(int8_t i=0; i<4; i++){

		// Desplazo y aplico máscara para obtener el estado del bit i
		uint8_t bit= (bcd_number_>>i) & 1; 

		if (bit==1){
			GPIOOn(gpio_map[i].pin);  // Accedo al campo .pin de la estructura
		}

		if (bit==0){
			GPIOOff(gpio_map[i].pin);
		}
	}

}

/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Guia 1 - Ejercicio 5\n");

	// Defino el mapeo usando la estructura que pide el enunciado 
    gpioConf_t mis_gpios[4] = {
        {GPIO_20, GPIO_OUTPUT},
        {GPIO_21, GPIO_OUTPUT},
        {GPIO_22, GPIO_OUTPUT},
        {GPIO_23, GPIO_OUTPUT}
    };

	// Inicializo usando los datos del vector
    for(int i=0; i<4; i++){
        GPIOInit(mis_gpios[i].pin, mis_gpios[i].dir);
    }

	uint8_t bcd_number = 0b1010; 
	GPIO_onoff(bcd_number, mis_gpios);

}
/*==================[end of file]============================================*/