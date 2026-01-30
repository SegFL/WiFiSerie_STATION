
#include "driver/uart.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "../configuration/configuration.h"

#define UART_NUM UART_NUM_0


void initUart();
void sendUartDataln(const char* data) ;
void sendUartData(const char* data) ;
void writeSerialComln(const char* data);
void writeSerialCom(const char* data);
void clearScreen();
char readSerialChar(void);
void updateUartBuffers();