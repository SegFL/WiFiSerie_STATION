#include "network.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "../configuration/configuration.h"

#define WIFI_CONNECTED_BIT BIT0

bool snifferEnabled=false;

bool isTcpSocketHealthy(int sock);

static const char *TAG = "ESP32_TCP_SERVER";
static EventGroupHandle_t wifi_event_group;

char addr_str[64];
int sock=-1;




//Buffer para la recepcion de la UART
char uart_buffer[5120];
char rx_buffer[5120];

bool connected=false;


//Indica si hay na conexion TCP activa o no
bool isConnected(){
    connected=isTcpSocketHealthy(sock);
    return connected;
}

//Devuelve el estado del sniffer
bool snifferIsEnabled(){
    return snifferEnabled;
}
//Devuelve el estado del sniffer y lo cambia si es necesario
bool snifferEnable(bool enable){
    snifferEnabled=enable;
    return snifferEnabled;
}
void networkInit() {
    xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CONNECTED_BIT,
        pdFALSE,
        pdTRUE,
        portMAX_DELAY
    );

    ESP_LOGI(TAG, "WiFi conectada, iniciando TCP SERVER");

}

void printNetworkInfo(){


    char screen_buf[256];   // ajustá el tamaño según tu pantalla

    
    writeSerialComln("=== INFORMACION DE RED ===");


    
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (!netif) return;

    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(netif, &ip_info);

    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);


        snprintf(screen_buf, sizeof(screen_buf),
            "------------- RED -------------\r\n"
            "IP(ESP32)   : " IPSTR "\r\n"
            "MASK        : " IPSTR "\r\n"
            "GW          : " IPSTR "\r\n"
            "MAC         : %02X:%02X:%02X:%02X:%02X:%02X\r\n"
            "TCP PORT    : %u\r\n"
            "Estado TCP  : %s\r\n"
            "--------------------------------\r\n",
            IP2STR(&ip_info.ip),
            IP2STR(&ip_info.netmask),
            IP2STR(&ip_info.gw),
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
            getTCPServerPort(),
            isConnected() ? "Conectado" : "Desconectado"
    );

    writeSerialCom(screen_buf);


    
    
    

}






/* ================= WIFI EVENTS ================= */

void wifi_event_handler(void *arg,
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

        printNetworkInfo();
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}



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
    strcpy((char *)wifi_config.sta.ssid, getWifiSSID());
    strcpy((char *)wifi_config.sta.password, getWifiPass());

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi_init_sta terminado");
}





/* ================= TCP SERVER ================= */

void tcp_server_task(void *pvParameters)
{




    wifi_init_sta();
    networkInit(); 

    int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Error creando socket");
        vTaskDelete(NULL);
        return;
    }
    
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(TCP_SERVER_PORT),//Pongo el puerto por defecto, luego lo cambio si hay alguno valido 
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };
    //Leo el puerto TCP desde la fonciguracion
    server_addr.sin_port = htons(getTCPServerPort());
    bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(listen_sock, 1);

    ESP_LOGI(TAG, "TCP SERVER escuchando en puerto %d", getTCPServerPort());

    while (1) {
        struct sockaddr_in client_addr; //Info del cliente
        socklen_t addr_len = sizeof(client_addr);

        ESP_LOGI(TAG, "Esperando cliente TCP...");
        sock = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len);
        if (sock < 0) continue;

        inet_ntop(AF_INET, &client_addr.sin_addr, addr_str, sizeof(addr_str));
        ESP_LOGI(TAG, "Cliente conectado desde %s", addr_str);

        connected=true;
        while (1) {
            //Recibe x cantidad de datos, si no hay suficientes datos devuelve lo disponible. Para configurar no bloqueante ver el ultimo argumento https://man7.org/linux/man-pages/man2/recv.2.html
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len <= 0) break;
            //send_uart()
            rx_buffer[len] = 0;
            //ESP_LOGI(TAG, "RX: %s", rx_buffer);
            ESP_LOGI(TAG, "TCP -> UART (%d bytes)", len);

            sendUartData(rx_buffer);
        }
        connected=false;
        ESP_LOGI(TAG, "Cliente desconectado");

        close(sock);
    }
}



//Funcion que se encarga de enviar los datos recibidos por UART al cliente TCP conectado
//Tiene implementado un envio parcial de datos y reintentos en caso de que el buffer TCP este lleno
//Si se llena el buffer TCP se espera 10ms y se reintenta, si nunca se vacia el buffer 
//se puede quedar bloqueado indefinidamente
void transmitUartTcp()
{






        if (!isConnected() || sock < 0 || !snifferEnabled) {
            vTaskDelay(pdMS_TO_TICKS(10));
            return;
        }

        //No bloqueante, si no hay datos disponibles retorna 0 y sigue
        int len = uart_read_bytes(
            UART_NUM_1,
            uart_buffer,
            sizeof(uart_buffer),
            0
        );

        if (len <= 0) {
            return;
        }

        int total_sent = 0;

        while (total_sent < len) {

            int ret = send(
                sock,
                uart_buffer + total_sent,
                len - total_sent,
                0
            );
            if(snifferEnabled==true){
                ESP_LOGI(TAG, "Enviando datos por TCP: %.*s", len, uart_buffer);
            }

            if (ret > 0) {
                total_sent += ret;
            }
            else if (ret < 0) {

                if (errno == EWOULDBLOCK || errno == ENOMEM) {
                    // Buffer TCP lleno → esperar
                    vTaskDelay(pdMS_TO_TICKS(10));
                    continue;
                } else {
                    ESP_LOGE(TAG, "send() error fatal (%d), cerrando conexion", errno);
                    connected = false;
                    close(sock);
                    sock = -1;
                    break;
                }
            }
        }

        ESP_LOGI(TAG, "UART -> TCP (%d bytes)", total_sent);
}











//Funcion para saber si el socket sigue vivo
bool isTcpSocketHealthy(int sock)
{
    if (sock < 0) {
        return false;
    }

    int error = 0;
    socklen_t len = sizeof(error);

    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
        return false;
    }

    // error == 0 → socket OK
    return (error == 0);
}






#define TCP_SERVER_PORT_FIXED 4000


static const char *TAG_E = "TCP_EXAMPLE";

void tcp_server_debuger_task(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str[64];
    int sock = -1;

    /* Espera a que haya WiFi */
    while (!isConnected()) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
    ESP_LOGI(TAG_E, "WiFi conectado, iniciando TCP server");

    int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0) {
        ESP_LOGE(TAG_E, "Error creando socket");
        vTaskDelete(NULL);
        return;
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(TCP_SERVER_PORT_FIXED),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };

    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        ESP_LOGE(TAG_E, "Error en bind");
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }

    listen(listen_sock, 1);
    ESP_LOGI(TAG_E, "TCP SERVER escuchando en puerto %d", TCP_SERVER_PORT_FIXED);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        ESP_LOGI(TAG_E, "Esperando cliente TCP...");
        sock = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len);
        if (sock < 0) {
            continue;
        }

        inet_ntop(AF_INET, &client_addr.sin_addr, addr_str, sizeof(addr_str));
        ESP_LOGI(TAG_E, "Cliente conectado desde %s", addr_str);

        while (1) {
            int len = recv(sock, rx_buffer, sizeof(rx_buffer), 0);
            if (len <= 0) {
                break;
            }

            /* Ignora datos recibidos */
            ESP_LOGI(TAG_E, "RX %d bytes -> %s", len, ESP32_NAME);

            /* Envía mensaje fijo */
            send(sock, ESP32_NAME, strlen(ESP32_NAME), 0);
        }

        ESP_LOGI(TAG_E, "Cliente desconectado");
        close(sock);
    }
}

