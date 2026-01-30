#include <stdio.h>
#include <string.h>
#include <errno.h>

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

#include "modulos/serialCom/serialCom.h"
#include "modulos/menuTree/menuTree.h"
#include "modulos/userInterface/userInterface.h"
#include "modulos/network/network.h"
#include "modulos/configuration/configuration.h"
/* ================= CONFIG ================= */



/* ========================================= */


/* ================= NETWORK INFO ================= */



static const char *TAG = "ESP32_TCP_SERVER2";



void task2(void * pvParameters){
    while(1){
        userInterfaceUpdate();
        transmitUartTcp();
        vTaskDelay(pdMS_TO_TICKS(100));
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



    initUart();
    userInterfaceInit();



    xTaskCreate(
        task2,
        "task2",
        4096,
        NULL,
        5,
        NULL
    );


    xTaskCreate(
        tcp_server_task,
        "tcp_server",
        4096,
        NULL,
        5,
        NULL
    );
}
















