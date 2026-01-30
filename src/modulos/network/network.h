


#include "../serialCom/serialCom.h"

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










/* ================= NETWORK INFO ================= */







void networkInit();
void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data);

void printNetworkInfo();
void wifi_init_sta(void);
void tcp_server_task(void *pvParameters);


bool isConnected();
void transmitUartTcp();
void printUartInfo();