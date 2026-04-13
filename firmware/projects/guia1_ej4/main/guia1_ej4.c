/*! @mainpage GUIA 1 - EJERCICIO 4
 *
 * @section genDesc General Description
 *
 * Escriba una función que reciba un dato de 32 bits,  la cantidad de dígitos de salida y 
 * un puntero a un arreglo donde se almacene los n dígitos. La función deberá convertir el 
 * dato recibido a BCD, guardando cada uno de los dígitos de salida en el arreglo pasado como puntero
 *
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

/** @fn int8_t  convertToBcdArray (uint32_t numerodecimal_, uint8_t digits_, uint8_t * bcd_number)
 * @brief Convierte un número decimal a su representación en BCD 
 * @param[in] numerodecimal_ Número decimal a convertir.
 * @param[in] digits_ Cantidad de dígitos de salida.
 * @param[out] bcd_number Puntero al arreglo donde se almacenarán los dígitos BCD.
 * @return 0 en caso de éxito, -1 en caso de error.
 */
int8_t  convertToBcdArray (uint32_t numerodecimal_, uint8_t digits_, uint8_t * bcd_number)
{
	if (bcd_number == NULL) {
		return -1;
	} // el puntero no puede ser nulo

	for (int8_t i = digits_-1; i>=0; i--){	// se lee de derecha a izquierda y cuenta desde el 0 
		bcd_number[i] = numerodecimal_ % 10;  //guardo el ultimo digito del numero en el array 
		numerodecimal_ /= 10;  //elimino el ultimo digito del numero 
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
