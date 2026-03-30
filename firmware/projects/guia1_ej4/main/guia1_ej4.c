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
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

uint8_t  convertToBcdArray (uint32_t numerodecimal_, uint8_t digits_, uint8_t * bcd_number)
{
	if (bcd_number == NULL) {
		return -1;
	} // el puntero no puede ser nulo

	for (int8_t i = digits_-1; i>=0; i--){	//se lee de derecha a izquierda y cuenta desde el 0
		bcd_number[i] = numerodecimal_ % 10; // guardo el ultimo digito del numero en el array
		numerodecimal_ /= 10; // elimino el ultimo digito del numero
	}

	if (numerodecimal_ != 0){
		return -1; // el numero tiene más digitos que el tamaño del array
	}

	return 0;
}

/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Guia1_ej4\n");
	uint32_t numerodecimal = 123;
	uint8_t digits = 3;
	uint8_t bcd_number[digits]; // se decide el tamaño en tiempo de ejecución, no en tiempo de compilación
	convertToBcdArray(numerodecimal, digits, bcd_number);

}
/*==================[end of file]============================================*/
