/*! @mainpage GUIA 1 - EJERCICIO 3
 *
 * @section genDesc General Description
 *
 * Realice un función que reciba un puntero a una estructura LED, y utilice como guía 
 * para la implementación el diagrama de flujo que se observa en la Figura 1 
 *
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_12	 	| 	GPIO_11		|
 * | 	PIN_11	 	| 	GPIO_10 	|
 * | 	PIN_5	 	| 	GPIO_5		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 18/03/2026 | Document creation		                         |
 *
 * @author Nazarena Romero (romeronaza030@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <freertos/FreeRTOS.h> // Incluyo la libreria de FreeRTOS para usar la funcion vTaskDelay
#include <freertos/task.h> // Incluyo la libreria de FreeRTOS para usar la funcion vTaskDelay
#include <led.h>
/*==================[macros and definitions]=================================*/
/*==================[internal data definition]===============================*/

/** @brief Estructura para almacenar la información de los leds 
*/
struct leds
{	
	uint8_t estado; /**< Estado del led: 1=prender , 2=apagar , 3=titilar */
	uint8_t num_led; /**< Número del led: LED_1, LED_2, LED_3 */
	uint8_t num_ciclos; /**< Número de ciclos de titilado (solo se usa si estado=3) */
	uint16_t periodo; /**< Periodo de titilado en milisegundos (solo se usa si estado=3) */
} my_leds; /**< nombre de la variable con los 4 campos disponibles */

/*==================[internal functions declaration]=========================*/

/** @fn void prender_apagar_titilar (struct leds *led_placa)
 * @brief Controla el comportamiento del LED (Encendido, Apagado o Titilado).
 * Esta función evalúa el campo @p estado del struct recibido y ejecuta
 * la acción correspondiente sobre el hardware.
 * @param[in,out] led_placa Puntero a la estructura que contiene la configuración del LED.
 * @note Si el estado es 3 (Titilar), la función bloqueará la tarea actual 
 * debido al uso de vTaskDelay() durante el tiempo especificado en el periodo.
 * @warning Asegúrese de que el puntero @p led_placa no sea NULL antes de invocar la función.
*/
void prender_apagar_titilar (struct leds *led_placa){ //la variable es de tipo "leds" y se llama *led_placa, el * indica su posicion de memoria
	
	switch (led_placa->estado){ //la flecha indica que dentro del struct quiere acceder al campo modo
		
		case 1:
			LedOn(led_placa->num_led);
		break;

		case 2:
			LedOff(led_placa->num_led);
		break; 

		case 3:

// Una opcion es que el led prenda en un ciclo (1s) y apague en el siguiente ciclo (1s). PARPADEA 3 VECES.
			for (int8_t i=0; i<led_placa->num_ciclos; i++){
				LedToggle(led_placa->num_led);
				vTaskDelay(pdMS_TO_TICKS(led_placa->periodo));
			}

/* Otra opcion es que el led en UN mismo ciclo (1s) prenda durante 0.5s y apague durante 0.5s. PARPADEA 5 VECES.
		for (uint8_t i=0; i<led_placa->num_ciclos; i++){
			LedToggle(led_placa->num_led);
			vTaskDelay(pdMS_TO_TICKS(led_placa->periodo/2));
			LedToggle(led_placa->num_led);
			vTaskDelay(pdMS_TO_TICKS(led_placa->periodo/2));
		} 
*/
		break;
	}
	
}

/*==================[external functions definition]==========================*/
void app_main(void){
	
	LedsInit(); //inicializo los leds

	my_leds.estado = 1; 
	my_leds.num_ciclos = 6;
	my_leds.periodo = 1000; //milisegundos

	for (int8_t i=1; i<=3; i++){
		my_leds.estado = i;  // 1=prender , 2=apagar , 3=titilar 

		for (int8_t j=1; j<=3; j++){
			if (j==1) my_leds.num_led = LED_1; 
			if (j==2) my_leds.num_led = LED_2; 
			if (j==3) my_leds.num_led = LED_3; 
			prender_apagar_titilar(&my_leds); /**<le paso la direccion de memoria de la variable my_leds, para que pueda acceder a sus campos*/
		}
	}

/*	my_leds.num_led = LED_1;
	prender_apagar_titilar(&my_leds);//le paso la direccion de memoria de la variable my_leds, para que pueda acceder a sus campos

	my_leds.num_led = LED_2; //prendo el led 2
	prender_apagar_titilar(&my_leds);

	my_leds.num_led = LED_3; //prendo el led 3
	prender_apagar_titilar(&my_leds);
*/

}


/*==================[end of file]============================================*/