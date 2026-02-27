




#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include <string.h>



#define LED_RUN GPIO_NUM_2   



#define WIFI_SSID           "ROUTER"
#define WIFI_PASS           "00112233"
#define TCP_SERVER_PORT     5000
#define TCP_DEBUG_PORT      4000
#define UART_BAUDRATE_DEF   9600
#define UART_BUF_SIZE_DEF   1024
#define TCP_BUF_SIZE_DEF    1024

void loadConfiguration(void);
//Wifi
    bool setWifiSSID(const char* ssid);
    bool setWifiPass(const char* pass);
//---
    const char* getWifiSSID(void);
    const char* getWifiPass(void);
//Tcp
    bool setTcpPort(uint16_t port);
//---
    uint16_t getTCPServerPort();
//Uart
    bool setUartBaudrate(uint32_t baudrate);
    bool setUartBufferSize(uint16_t size);
//---
    uint32_t getUartBaudrate();
    uint16_t getUartBufferSize();



