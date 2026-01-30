
#include "serialCom.h"

uart_config_t uart_config1;
uart_config_t uart_config2;


void moveCursor(int row, int col) ;

void initUart(void)
{
    uint32_t baud = getUartBaudrate();
    uint16_t buf  = getUartBufferSize();

    // ===== UART0 (CONSOLA) =====
    uart_config1 = (uart_config_t){
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0
    };

    uart_param_config(UART_NUM_0, &uart_config1);
    uart_driver_install(UART_NUM_0, 1024, 1024, 0, NULL, 0);

    sendUartDataln("UART0 (CONSOLA) iniciada");

    // ===== UART1 (DUT) =====
    uart_config2 = (uart_config_t){
        .baud_rate = baud,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0
    };

    uart_param_config(UART_NUM_1, &uart_config2);
    uart_driver_install(UART_NUM_1, buf, buf, 0, NULL, 0);

    sendUartDataln("UART1 (DUT) iniciada");
}



void sendUartDataln(const char* data) {
    sendUartData( data);
    sendUartData( "\n\r");
}


void sendUartData(const char* data) {
    uart_write_bytes(UART_NUM_1, data, strlen(data));

}



void writeSerialCom(const char* data){
    uart_write_bytes(UART_NUM_0, data, strlen(data));
}

void writeSerialComln(const char* data){
    uart_write_bytes(UART_NUM_0, data, strlen(data));
    uart_write_bytes(UART_NUM_0, "\n\r", 2);
}



void clearScreen() {
    writeSerialCom("\033[2J\033[H");  // Borra pantalla ANSI
}






//Funcion para leer de a un caracter de la CONSOLA
char readSerialChar(void)
{
    uint8_t ch;
    
    // Intentar leer 1 byte sin bloquear
    int len = uart_read_bytes(
        UART_NUM_0,
        &ch,
        1,
        0    // timeout = 0 → NO bloqueante
    );

    if (len > 0) {
        // Loopback (eco)
        uart_write_bytes(UART_NUM_0, (const char *)&ch, 1);

        // Filtrar '\r'
        if (ch == '\r') {
            return '\0';
        }

        return (char)ch;
    }

    return '\0';
}


void printUartInfo(void)
{
    char out[256];

    uint16_t buf_size = getUartBufferSize();
    size_t rx_used = 0;

    // Bytes actualmente almacenados en RX
    uart_get_buffered_data_len(UART_NUM_1, &rx_used);

    snprintf(out, sizeof(out),
             "=== INFORMACION DE UART (DUT) ===\r\n"
             "Baudrate      : %lu\r\n"
             "Data bits     : %d\r\n"
             "Parity        : %s\r\n"
             "Stop bits     : %d\r\n"
             "Flow control  : %s\r\n"
             "RX buffer     : %u bytes (usados:  )\r\n"
             "TX buffer     : %u bytes (libres:  )\r\n"
             "--------------------------------\r\n",
             (unsigned long)uart_config2.baud_rate,
             uart_config2.data_bits + 5,
             (uart_config2.parity == UART_PARITY_DISABLE) ? "None" :
             (uart_config2.parity == UART_PARITY_EVEN)    ? "Even" : "Odd",
             (uart_config2.stop_bits == UART_STOP_BITS_1) ? 1 : 2,
             (uart_config2.flow_ctrl == UART_HW_FLOWCTRL_DISABLE) ? "None" : "RTS/CTS",
             (unsigned int)buf_size,
             (unsigned int)buf_size
             
    );

    writeSerialCom(out);
    updateUartBuffers();
}


void updateUartBuffers(){

    size_t rx_used = 0;
    size_t tx_free = 0;
    // Bytes actualmente almacenados en RX
    uart_get_buffered_data_len(UART_NUM_1, &rx_used);
    uart_get_tx_buffer_free_size(UART_NUM_1, &tx_free);
    char out1[20];
    snprintf(out1, sizeof(out1), "%u bytes )", (unsigned int)rx_used);
    moveCursor(8,36);
    writeSerialCom(out1);
    char out2[20];
    snprintf(out2, sizeof(out2), "%u bytes )", (unsigned int)tx_free);
    moveCursor(9,36);
    writeSerialCom(out2);


    moveCursor(11,0);
}

void moveCursor(int row, int col) {
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "\033[%d;%dH", row, col);
    writeSerialCom(buffer);

}