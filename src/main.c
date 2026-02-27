#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/inet.h"

#include "driver/uart.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "modulos/serialCom/serialCom.h"
#include "modulos/menuTree/menuTree.h"
#include "modulos/userInterface/userInterface.h"
#include "modulos/network/network.h"
#include "modulos/configuration/configuration.h"















static const char *TAG = "ESP32_TCP_SERVER2";

void ledsInit();
void led_task(void *pvParameters);
void comunication_task(void *pvParameters);

void ui_task(void * pvParameters)
{


    while(1)
    {
        networkDebugPoll();   
        userInterfaceUpdate();

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}



/* ================= MAIN ================= */

void app_main(void)
{


    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }
    //Carga cofiguracion desde NVS como SSID, PASS, etc.
    loadConfiguration();

    ledsInit();


    initUart();
    wifi_init_sta();
    // Si no tengo conexion no hago nada mas
    while (!isWifiConnected()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    networkDebugInit();
    userInterfaceInit();


    //Tarea asociada a todo lo relacionado con la interfaz de usuario como
    //la UART0, el menu y la conexion TCP de debug (port 4000)
    xTaskCreate(
        ui_task,
        "ui_task",
        4096,
        NULL,
        5,
        NULL
    );





    xTaskCreate(
        comunication_task,
        "comunication_task",
        4096,
        NULL,
        5,
        NULL
    );

    


    xTaskCreate(
        led_task,
        "led_task",
        1024,
        NULL,
        5,
        NULL
    );







    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }


}




void ledsInit()
{
    gpio_reset_pin(LED_RUN);
    gpio_set_direction(LED_RUN, GPIO_MODE_OUTPUT);

}



void led_task(void *pvParameters)
{
    while (1) {
        if (isWifiConnected())
                {
                    // ----- ESTADO CONECTADO -----
                    gpio_set_level(LED_RUN, 0);
                    vTaskDelay(pdMS_TO_TICKS(500));

                    gpio_set_level(LED_RUN, 1);
                    vTaskDelay(pdMS_TO_TICKS(125));

                    gpio_set_level(LED_RUN, 0);
                    vTaskDelay(pdMS_TO_TICKS(125));

                    gpio_set_level(LED_RUN, 1);
                    vTaskDelay(pdMS_TO_TICKS(125));

                    gpio_set_level(LED_RUN, 0);
                    vTaskDelay(pdMS_TO_TICKS(125));
                }
                else
                {
                    // ----- ESTADO NORMAL -----
                    gpio_set_level(LED_RUN, 1);
                    vTaskDelay(pdMS_TO_TICKS(500));

                    gpio_set_level(LED_RUN, 0);
                    vTaskDelay(pdMS_TO_TICKS(500));
                }
            }
}






void comunication_task(void *pvParameters){

    while(true){
        transmitTcpUart();
        transmitUartTcp();

    }
}

