#include "configuration.h"

static const char *CFG_TAG = "CONFIG";


char g_wifi_ssid[32];
char g_wifi_pass[64];
//Los puertos TCP van de 0 a 65535(16bits)
uint16_t g_tcp_port;



uint32_t g_uart_baudrate;
uint16_t g_uart_buf_size;
uint16_t g_tcp_buf_size;




bool setWifiSSID(const char* ssid)
{
    esp_err_t err;
    nvs_handle_t nvs;

    err = nvs_open("config", NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        ESP_LOGE(CFG_TAG, "Error abriendo NVS para escribir SSID");
        return false;
    }

    err = nvs_set_str(nvs, "ssid", ssid);
    if (err != ESP_OK) {
        ESP_LOGE(CFG_TAG, "Error escribiendo SSID en NVS");
        nvs_close(nvs);
        return false;
    }

    nvs_commit(nvs);
    nvs_close(nvs);

    ESP_LOGI(CFG_TAG, "SSID guardado en NVS: %s", ssid);
    return true;
}

bool setWifiPass(const char* pass)
{
    esp_err_t err;
    nvs_handle_t nvs;

    err = nvs_open("config", NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        ESP_LOGE(CFG_TAG, "Error abriendo NVS para escribir PASS");
        return false;
    }

    err = nvs_set_str(nvs, "pass", pass);
    if (err != ESP_OK) {
        ESP_LOGE(CFG_TAG, "Error escribiendo PASS en NVS");
        nvs_close(nvs);
        return false;
    }

    nvs_commit(nvs);
    nvs_close(nvs);

    ESP_LOGI(CFG_TAG, "PASS guardado en NVS");
    return true;
}

bool setTcpPort(uint16_t port)
{
    esp_err_t err;
    nvs_handle_t nvs;

    err = nvs_open("config", NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        ESP_LOGE(CFG_TAG, "Error abriendo NVS para escribir TCP PORT");
        return false;
    }

    err = nvs_set_u16(nvs, "tcp_port", (uint16_t)port);
    if (err != ESP_OK) {
        ESP_LOGE(CFG_TAG, "Error escribiendo TCP PORT en NVS");
        nvs_close(nvs);
        return false;
    }

    nvs_commit(nvs);
    nvs_close(nvs);

    ESP_LOGI(CFG_TAG, "TCP PORT guardado en NVS: %d", port);
    return true;
}

bool setUartBaudrate(uint32_t baudrate)
{
    if (baudrate < 1200 || baudrate > 2000000) {
        ESP_LOGE(CFG_TAG, "Baudrate fuera de rango");
        return false;
    }

    esp_err_t err;
    nvs_handle_t nvs;

    err = nvs_open("config", NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        ESP_LOGE(CFG_TAG, "Error abriendo NVS para baudrate");
        return false;
    }

    err = nvs_set_u32(nvs, "baud", baudrate);
    if (err != ESP_OK) {
        ESP_LOGE(CFG_TAG, "Error escribiendo baudrate");
        nvs_close(nvs);
        return false;
    }

    nvs_commit(nvs);
    nvs_close(nvs);

    g_uart_baudrate = baudrate;
    ESP_LOGI(CFG_TAG, "UART baudrate guardado: %lu", baudrate);

    return true;
}


bool setUartBufferSize(uint16_t size)
{
    if (size < 128 || size > 16384) {
        ESP_LOGE(CFG_TAG, "UART buffer size invalido");
        return false;
    }

    esp_err_t err;
    nvs_handle_t nvs;

    err = nvs_open("config", NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        ESP_LOGE(CFG_TAG, "Error abriendo NVS para UART buffer");
        return false;
    }

    err = nvs_set_u16(nvs, "uart_buf", size);
    if (err != ESP_OK) {
        ESP_LOGE(CFG_TAG, "Error escribiendo UART buffer");
        nvs_close(nvs);
        return false;
    }

    nvs_commit(nvs);
    nvs_close(nvs);

    g_uart_buf_size = size;
    ESP_LOGI(CFG_TAG, "UART buffer guardado: %d", size);

    return true;
}



void loadConfiguration(void)
{
    esp_err_t err;
    nvs_handle_t nvs;

    /* ---------- Defaults ---------- */
    strcpy(g_wifi_ssid, WIFI_SSID);
    strcpy(g_wifi_pass, WIFI_PASS);
    g_tcp_port      = TCP_SERVER_PORT;
    g_uart_baudrate = UART_BAUDRATE_DEF;
    g_uart_buf_size = UART_BUF_SIZE_DEF;
    g_tcp_buf_size  = TCP_BUF_SIZE_DEF;

    err = nvs_open("config", NVS_READONLY, &nvs);
    if (err != ESP_OK) {
        ESP_LOGW("CFG", "NVS no disponible, usando defaults");
        return;
    }

    /* ---------- WIFI ---------- */
    size_t len = sizeof(g_wifi_ssid);
    nvs_get_str(nvs, "ssid", g_wifi_ssid, &len);

    len = sizeof(g_wifi_pass);
    nvs_get_str(nvs, "pass", g_wifi_pass, &len);

    /* ---------- TCP ---------- */
    nvs_get_u16(nvs, "tcp_port", &g_tcp_port);

    /* ---------- UART ---------- */
    nvs_get_u32(nvs, "baud", &g_uart_baudrate);
    nvs_get_u16(nvs, "uart_buf", &g_uart_buf_size);
    nvs_get_u16(nvs, "tcp_buf", &g_tcp_buf_size);

    nvs_close(nvs);

    /* ---------- Validaciones ---------- */
    if (g_uart_baudrate < 1200 || g_uart_baudrate > 2000000)
        g_uart_baudrate = UART_BAUDRATE_DEF;

    if (g_uart_buf_size < 128 || g_uart_buf_size > 16384)
        g_uart_buf_size = UART_BUF_SIZE_DEF;

    if (g_tcp_buf_size < 128 || g_tcp_buf_size > 16384)
        g_tcp_buf_size = TCP_BUF_SIZE_DEF;

    ESP_LOGI("CFG", "Config cargada:");
    ESP_LOGI("CFG", "SSID: %s", g_wifi_ssid);
    ESP_LOGI("CFG", "TCP port: %d", g_tcp_port);
    ESP_LOGI("CFG", "UART baud: %lu", g_uart_baudrate);
    ESP_LOGI("CFG", "UART buf: %d", g_uart_buf_size);
    ESP_LOGI("CFG", "TCP buf: %d", g_tcp_buf_size);
}


uint16_t getTCPServerPort()
{
    return g_tcp_port;
}

uint32_t getUartBaudrate()
{
    return g_uart_baudrate;
}


uint16_t getUartBufferSize()
{
    return g_uart_buf_size;
}


const char* getWifiSSID(void)
{
    return g_wifi_ssid;
}

const char* getWifiPass(void)
{
    return g_wifi_pass;
}
