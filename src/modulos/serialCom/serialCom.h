
#include "driver/uart.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "../configuration/configuration.h"

//Uart de debug
#define UART_NUM UART_NUM_0


void initUart();
void sendUartDataln(const uint8_t* data, size_t len) ;
void sendUartData(const uint8_t* data, size_t len)  ;
void writeSerialComln(const char* data);
void writeSerialCom(const char* data);
void clearScreen();
char readSerialChar(void);
void updateUartBuffers();