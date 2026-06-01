
#include "serialCom.h"
#include "../network/network.h"
uart_config_t uart_config1;
uart_config_t uart_config2;


//Puerto UART para consola (logs, info, debug)
#define UART_DEBUG UART_NUM_0

//Puerto UART para comunicacion con DUT (Device Under Test)
#define UART_MAIN UART_NUM_2
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

    uart_param_config(UART_DEBUG, &uart_config1);
    uart_driver_install(UART_DEBUG, 1024, 1024, 0, NULL, 0);

    writeSerialComln("CONSOLA) iniciada");



    // =========================
    // UART2 (DUT / Puente TCP)
    // =========================
    uart_config2 = (uart_config_t){
        .baud_rate = baud,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0

    };
    uart_param_config(UART_MAIN, &uart_config2);

    // ** ASIGNAR LOS PINES 16/17 **
    uart_set_pin(
        UART_MAIN,
        16,                //TX
        17,              //RX
        UART_PIN_NO_CHANGE, // RTS → no usado
        UART_PIN_NO_CHANGE  // CTS → no usado
    );

    uart_driver_install(UART_MAIN, buf, buf, 0, NULL, 0);

    writeSerialComln("UART2 (DUT) iniciada");


}



void sendUartDataln(const uint8_t* data, size_t len) {
    static const uint8_t crlf[2] = { '\r', '\n' };

    sendUartData(data, len);
    sendUartData(crlf, sizeof(crlf));
}



void sendUartData(const uint8_t* data, size_t len) {
    uart_write_bytes(UART_MAIN, data, len);

    ESP_LOGI("BRIDGE", "TCP -> UART (%d bytes)", len);
    ESP_LOG_BUFFER_HEX("BRIDGE", data, len);


}



void writeSerialCom(const char* data)
{
    uart_write_bytes(UART_DEBUG, data, strlen(data));

    if (networkDebugIsConnected())
    {
        networkDebugSend(data, strlen(data));
    }
}

void writeSerialComln(const char* data){
    writeSerialCom(data);
    writeSerialCom("\n\r");
}



void clearScreen() {
    writeSerialCom("\033[2J\033[H");  // Borra pantalla ANSI
}






//Funcion para leer de a un caracter de la CONSOLA

// Función para leer un caracter de la consola o TCP
char readUserChar(void)
{
    uint8_t ch;

    // 1️⃣ Intentar leer de UART
    int len = uart_read_bytes(UART_DEBUG, &ch, 1, 0);
    if (len > 0) {
        uart_write_bytes(UART_DEBUG, (const char *)&ch, 1);
        if (ch == '\r') return '\0';
        return (char)ch;
    }

    // 2️⃣ Intentar leer del buffer TCP debug
    if (networkDebugReadByte(&ch)) {
        // Opcional: eco por UART también
        uart_write_bytes(UART_DEBUG, (const char *)&ch, 1);
        if (ch == '\r') return '\0';
        return (char)ch;
    }

    // 3️⃣ No hay datos disponibles
    return '\0';
}

void printUartInfo(void)
{
    char out[256];

    uint16_t buf_size = getUartBufferSize();
    size_t rx_used = 0;

    // Bytes actualmente almacenados en RX
    uart_get_buffered_data_len(UART_MAIN, &rx_used);

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
    uart_get_buffered_data_len(UART_DEBUG, &rx_used);
    uart_get_tx_buffer_free_size(UART_DEBUG, &tx_free);
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




