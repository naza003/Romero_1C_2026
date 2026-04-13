/*! @mainpage GUIA 1 - EJERCICIO 6
 *
 * @section genDesc General Description
 *
 * Escriba una función que reciba un dato de 32 bits,  la cantidad de dígitos de salida y dos vectores de 
 * estructuras del tipo  gpioConf_t. Uno  de estos vectores es igual al definido en el punto anterior y 
 * el otro vector mapea los puertos con el dígito del LCD a donde mostrar un dato:
 * Dígito 1 -> GPIO_19
 * Dígito 2 -> GPIO_18
 * Dígito 3 -> GPIO_9
 * La función deberá mostrar por display el valor que recibe. Reutilice las funciones creadas en el punto 4 y 5. 
 * Realice la documentación de este ejercicio usando Doxygen
 *
 *
 * @section hardConn Hardware Connection
 *
 * |   Peripheral   |  ESP32-C6 (GPIO) |  Pin Físico (Placa) |
 * |:--------------:|:----------------:|:-------------------:|
 * |   D1 (LSB)     |      GPIO_20     |        PIN_18       |
 * |   D2           |      GPIO_21     |        PIN_19       |
 * |   D3           |      GPIO_22     |        PIN_20       |
 * |   D4 (MSB)     |      GPIO_23     |        PIN_21       |
 * |   SEL_1        |      GPIO_19     |        PIN_17       |
 * |   SEL_2        |      GPIO_18     |        PIN_16       |
 * |   SEL_3        |      GPIO_9      |        PIN_15       |
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
#include <freertos/FreeRTOS.h> // Incluyo la libreria de FreeRTOS para usar la funcion vTaskDelay
#include <freertos/task.h> // Incluyo la libreria de FreeRTOS para usar la funcion vTaskDelay

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

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


/** @fn int8_t ConversionYMultiplexado (int8_t numerodecimal_, uint8_t digitos_, uint8_t *bcd_number, gpioConf_t *gpio_map, gpioConf_t *gpio_pulso)
 * @brief Convierte un número decimal a BCD y controla el estado de los GPIOs para multiplexar la salida en displays de 7 segmentos.
 * @param[in] numerodecimal_ Número decimal a convertir y mostrar.
 * @param[in] digitos_ Número de dígitos del número decimal (tamaño del vector bcd_number).
 * @param[out] bcd_number Puntero al array donde se almacenará el número en formato BCD.
 * @param[in] gpio_map Vector de estructuras que mapea los pines de datos (D1-D4).
 * @param[in] gpio_pulso Vector de estructuras que mapea los pines de selección (SEL1-SEL3)
 * @return 0 en caso de éxito, -1 en caso de error.
 */

int8_t ConversionYMultiplexado (int8_t numerodecimal_, uint8_t digitos_, uint8_t *bcd_number, gpioConf_t *gpio_map, gpioConf_t *gpio_pulso) //dos vectores de estructuras
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

		// Envio los digitos al gpio 20 a 23 , osea pone el dato (D1-D4) en el bus
		GPIO_onoff(bcd_number[j], gpio_map);

		//Enciendo el display correspondiente
		GPIOOn(gpio_pulso[j].pin);

		//Espero un tiempo para que el display muestre el numero
		vTaskDelay(pdMS_TO_TICKS(5)); // 5ms es un tiempo razonable para que el display muestre el numero

		//Apago el display para mostrar el siguiente numero
		GPIOOff(gpio_pulso[j].pin);
	}

	return 0; // la funcion se ejecuto correctamente
}



/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Guia1_ej6\n");

	// Defino el mapeo usando la estructura que pide el enunciado 
    gpioConf_t mis_gpios[7] = {
        {GPIO_20, GPIO_OUTPUT},
        {GPIO_21, GPIO_OUTPUT},
        {GPIO_22, GPIO_OUTPUT},
        {GPIO_23, GPIO_OUTPUT},
		{GPIO_19, GPIO_OUTPUT},
		{GPIO_18, GPIO_OUTPUT},
		{GPIO_9, GPIO_OUTPUT}
    };
	
	// Inicializo usando los datos del vector
    for(int i=0; i<7; i++){
        GPIOInit(mis_gpios[i].pin, mis_gpios[i].dir);
    }

	uint8_t numerodecimal = 123;
	uint8_t digitos = 3;
	uint8_t bcd_number [digitos];  

	// Defino los vectores de estructuras para pasar a la funcion
	gpioConf_t gpio_map[4] = {
		{GPIO_20, GPIO_OUTPUT}, 
		{GPIO_21, GPIO_OUTPUT}, 
		{GPIO_22, GPIO_OUTPUT}, 
		{GPIO_23, GPIO_OUTPUT}}; 

	gpioConf_t gpio_pulso[3] = {
		{GPIO_19, GPIO_OUTPUT}, 
		{GPIO_18, GPIO_OUTPUT}, 
		{GPIO_9, GPIO_OUTPUT}};

	int8_t tiempo = 0;
	while (tiempo < 5000){
		ConversionYMultiplexado(numerodecimal, digitos, bcd_number, gpio_map, gpio_pulso);
		tiempo++;
	}
}
/*==================[end of file]============================================*/