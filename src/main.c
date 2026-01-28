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

/* ================= CONFIG ================= */

#define WIFI_SSID      "LABOE"
#define WIFI_PASS      "LORTOTUA"

#define TCP_SERVER_PORT 5000

static const char *TAG = "ESP32_TCP_SERVER";

/* ========================================= */

static void print_network_info(void);
static EventGroupHandle_t wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

/* ================= WIFI EVENTS ================= */

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Conectando a WiFi...");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "WiFi desconectado, reintentando...");
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "IP obtenida: " IPSTR, IP2STR(&event->ip_info.ip));

        print_network_info();
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/* ================= WIFI INIT ================= */

void wifi_init_sta(void)
{
    wifi_event_group = xEventGroupCreate();

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);

    wifi_config_t wifi_config = { 0 };
    strcpy((char *)wifi_config.sta.ssid, WIFI_SSID);
    strcpy((char *)wifi_config.sta.password, WIFI_PASS);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi_init_sta terminado");
}

/* ================= NETWORK INFO ================= */

static void print_network_info(void)
{
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (!netif) return;

    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(netif, &ip_info);

    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);

    ESP_LOGI(TAG, "------------- RED -------------");
    ESP_LOGI(TAG, "IP      : " IPSTR, IP2STR(&ip_info.ip));
    ESP_LOGI(TAG, "MASK    : " IPSTR, IP2STR(&ip_info.netmask));
    ESP_LOGI(TAG, "GW      : " IPSTR, IP2STR(&ip_info.gw));
    ESP_LOGI(TAG, "MAC     : %02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    ESP_LOGI(TAG, "--------------------------------");
}

/* ================= TCP SERVER ================= */

static void tcp_server_task(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str[64];

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(TCP_SERVER_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };

    int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Error creando socket");
        vTaskDelete(NULL);
        return;
    }

    bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(listen_sock, 1);

    ESP_LOGI(TAG, "TCP SERVER escuchando en puerto %d", TCP_SERVER_PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        ESP_LOGI(TAG, "Esperando cliente TCP...");
        int sock = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len);
        if (sock < 0) continue;

        inet_ntop(AF_INET, &client_addr.sin_addr, addr_str, sizeof(addr_str));
        ESP_LOGI(TAG, "Cliente conectado desde %s", addr_str);

        while (1) {
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len <= 0) break;

            rx_buffer[len] = 0;
            ESP_LOGI(TAG, "RX: %s", rx_buffer);

            send(sock, rx_buffer, len, 0);  // eco
        }

        ESP_LOGI(TAG, "Cliente desconectado");
        close(sock);
    }
}

/* ================= MAIN ================= */

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    wifi_init_sta();

    xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CONNECTED_BIT,
        pdFALSE,
        pdTRUE,
        portMAX_DELAY
    );

    ESP_LOGI(TAG, "WiFi conectada, iniciando TCP SERVER");

    xTaskCreate(
        tcp_server_task,
        "tcp_server",
        4096,
        NULL,
        5,
        NULL
    );
}
