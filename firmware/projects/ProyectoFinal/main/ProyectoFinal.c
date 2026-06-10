/*! @mainpage Proyecto Final
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	| Dispositive |
 * |:--------------:|:--------------|-------------|
 * | 	PIN_X	 	| 	GPIO_8		|  NeoPixel   |
 * | 	CH1		 	| 	GPIO_1		|  MP3V5050   |
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 24/05/2026 | Document creation		                         |
 *
 * @author Nazarena Romero (romeronaza030@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <analog_io_mcu.h>
#include <uart_mcu.h>
#include <timer_mcu.h>
#include <neopixel_stripe.h>
#include <mp3v5050.h>
#include <switch.h>

/*==================[macros and definitions]=================================*/

#define periodo_muestreo 100000 // 100 ms

//umbrales seguros para el dispositivo
#define umbral_max_kPa 100
#define umbral_min_kPa 20

//definidos segun lo sensado en el paciente o segun lo definido por bibliografía
#define umbral_1_kPa 0 //reposo + 10-20% para medir inicio de espiración 
#define umbral_2_kPa 50 // reposo + 50-70%
#define umbral_3_kPa 80 // reposo + 80-90%
#define umbral_4_kPa 100 // reposo + 100% para evitar esfuerzo excesivo


#define CONTROL_DELAY_MS 1000
bool medir_presion = false; 
static neopixel_color_t pixel_data[12];


/*==================[internal data definition]===============================*/
TaskHandle_t Sensado_task_handle = NULL;
TaskHandle_t LeerTeclas_task_handle = NULL;

/*==================[internal functions declaration]================================*/

static neopixel_color_t mapeoPresionAColor(float presion_kPa){

    if (presion_kPa < 0.3f) {
        return NEOPIXEL_COLOR_RED;
    } else if (presion_kPa < 0.8f) {
        return NEOPIXEL_COLOR_ORANGE;
    } else if (presion_kPa < 1.3f) {
        return NEOPIXEL_COLOR_YELLOW;
    } else if (presion_kPa < 1.8f) {
        return NEOPIXEL_COLOR_GREEN;
    } else {
        return NEOPIXEL_COLOR_BLUE;
    }
}

static void ControlNeoPixel(neopixel_color_t color){
    NeoPixelAllOff();
	NeoPixelAllColor(color);
}

static void Sensado(void *pvParameters){
    while (true) {
        if (medir_presion==true) {
            float presion_medida = MP3v5050ReadPressure_kPa();
            neopixel_color_t color = mapeoPresionAColor(presion_medida);
            ControlNeoPixel(color);
        } else {
            NeoPixelAllOff();
        }
        vTaskDelay(CONTROL_DELAY_MS / portTICK_PERIOD_MS);
    }
}

static void LeerTeclas(void *pvParameter){

    while(true){

        int8_t teclas = SwitchesRead();  //leo el estado de las teclas

        switch(teclas){

            case SWITCH_1:
                medir_presion = !medir_presion; //si se presiona la tecla 1, se activa o desactiva la medición
                break;

            default:
                break;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}


/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");

	MP3v5050Init();
	SwitchesInit();
	NeoPixelInit(BUILT_IN_RGB_LED_PIN, 12, pixel_data);

	xTaskCreate(&Sensado, "Sensado", 2048, NULL, 5, &Sensado_task_handle);
    xTaskCreate(&LeerTeclas, "LeerTeclas", 2048, NULL, 5, &LeerTeclas_task_handle);
}
/*==================[end of file]============================================*/