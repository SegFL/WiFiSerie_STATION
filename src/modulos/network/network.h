


#include "../serialCom/serialCom.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"



#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/inet.h"

#include "driver/uart.h"
#include "esp_log.h"









/*======================================================= NOMBRE UNICO DEL DISPOSITIVO ===================================================*/
#define ESP32_NAME     "ESP32-ASTERISCO3\n\r"
/*========================================================================================================================================*/








/* ================= NETWORK INFO ================= */









void networkInit();
void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data);

void printNetworkInfo();
void wifi_init_sta(void);
void transmitTcpUart();


bool isConnected();
void transmitUartTcp();
void printUartInfo();
void tcp_server_debuger_task(void *pvParameters);
void sendSequenceDebug(int sock);
void enabledTCPDebugSequence(bool enabled);
bool isWifiConnected(void);
bool isTcpConnected();
void enabledUARTDebugSequence(bool enabled);
void sendUARTSequenceDebug();




//Funciones relacionados a la conexion TCP de debug
void networkDebugInit(void);
void networkDebugPoll(void);

bool networkDebugIsConnected(void);
void networkDebugSend(const char *data, int len);
bool networkDebugReadByte(uint8_t* ch);
