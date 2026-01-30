
#include <ctype.h>
#include "esp_wifi.h"
#include <string.h>
#include "userInterface.h"
#include "modulos/serialCom/serialCom.h"
#include "modulos/menuTree/menuTree.h"
#include "modulos/network/network.h"


#define MAX_DATA_BUFFER 30


static MenuNode *menu = NULL;
char data_buffer[MAX_DATA_BUFFER] ; //Variable para almacenar los datos recibidos
unsigned char buffer_index = 0; //Índice para el buffer de datos
bool aceptandoDatos=false;
bool updateScreen=false;

static int lastMenuId = -1;



void moveCursor(int row, int col);

void printSensorData();


void procesarDatos(const char* data, unsigned char length) ;


static void onEnterNode(MenuNode* n);
static void onUpdateNode(MenuNode* n);
static bool nodeRequiresInput(int id);

void printSavedCurves();
void printAllCurvesNvs();
void printSensorInfo();



void userInterfaceInit(){

    clearScreen();//Borra mensajes del ESP32 al iniciar el programa
    menu=menuInit();
    if(menu){
        writeSerialComln("Menu inicializado");
    }else{
        writeSerialComln("Menu no inicializado");
    }


}


void userInterfaceUpdate() {
    if (menu == NULL) return;

    char charReceived = readSerialChar();
    if(charReceived== GO_BACK){ // ESCAPE
        menuUpdate(charReceived, &menu);
        
        clearScreen();
        printNode(menu);
        onEnterNode(menu);
        lastMenuId = menu->id;
        
        //Si el nodo es nuevo y requiere datos, preparo el buffer para recibirlos
        //Si no es nuevo pero aun asi requiere datos(porque ya se enviaron datos previamente
        //y se quiere seguir enviando datos) tambien preparo el buffer
        aceptandoDatos = nodeRequiresInput(menu->id);

        
        return ;
    }
  

    if (charReceived == '\n') {
        if (aceptandoDatos) {
            // Terminamos de recibir datos
            data_buffer[buffer_index] = '\0';  // Terminador nulo
            procesarDatos(data_buffer,buffer_index);
            memset(data_buffer, 0, sizeof(data_buffer));
            buffer_index = 0;
            aceptandoDatos = false;
        } 
        return;
    }

   if (!aceptandoDatos) {

        
        menuUpdate(charReceived, &menu);

        if (lastMenuId != menu->id) {
            clearScreen();
            printNode(menu);
            onEnterNode(menu);
            lastMenuId = menu->id;
        }
        //Si el nodo es nuevo y requiere datos, preparo el buffer para recibirlos
        //Si no es nuevo pero aun asi requiere datos(porque ya se enviaron datos previamente
        //y se quiere seguir enviando datos) tambien preparo el buffer
        aceptandoDatos = nodeRequiresInput(menu->id);

    } else {
        // Captura de caracteres
        if(buffer_index < MAX_DATA_BUFFER - 1) {
            // Aceptamos solo números,caracteres , coma y espacios
            if (isdigit(charReceived) || isalpha(charReceived) || charReceived == ',' || isspace(charReceived)) {
                data_buffer[buffer_index++] = charReceived;
            } 
        }
    }




    // 🔹 Ejecutar siempre la lógica de actualización periódica
    onUpdateNode(menu);

    return;
}








void procesarDatos(const char* data, unsigned char length) {
    if (data == NULL || menu == NULL || length <= 0) { 
        return;
    }


   /* ===== CAMBIAR PUERTO TCP ===== */

    if(menu->id == 12){
        //Cambio SSID
        writeSerialComln("Cambiando SSID a:");
        writeSerialComln(data);
        if(strlen(data)<1 || strlen(data)>31){
            writeSerialComln("SSID invalido. Debe tener entre 1 y 31 caracteres");
            return;
        }
        if(setWifiSSID(data)){
            writeSerialComln("SSID cambiado correctamente. Reinicie para aplicar cambios");
        }else{
            writeSerialComln("Error al cambiar SSID");
        }
        return;
    }

    if(menu->id == 13){
        //Cambio SSID
        if(strlen(data)<1 || strlen(data)>63){
            writeSerialComln("SSID invalido. Debe tener entre 1 y 31 caracteres");
            return;
        }
        if(setWifiPass(data)){
            writeSerialComln("Contraseña cambiada correctamente. Reinicie para aplicar cambios");
        }else{
            writeSerialComln("Error al cambiar contraseña");
        }
        return;
    }

    if (menu->id == 14) {
        int port;
        int parsed;
        /* sscanf funciona bien para este caso */
        parsed = sscanf(data, "%d", &port);
        if (parsed != 1) {
            writeSerialCom("Error: ingrese un numero valido\r\n");
            return;
        }
        if (port < 0 || port > 65535) {
            writeSerialCom("Puerto fuera de rango (1024-65535)\r\n");
            return;
        }
        if (!setTcpPort((uint16_t)port)) {
            writeSerialCom("Error al guardar puerto TCP\r\n");
            return;
        }
        writeSerialCom("Puerto TCP guardado correctamente\r\n");
        writeSerialCom("Reinicie el equipo para aplicar cambios\r\n");
        return;
    }


    /* ======= Cambiar UART ======*/

    if (menu->id == 22) {

    uint32_t baud;
    if (sscanf(data, "%lu", &baud) != 1) {
        writeSerialCom("Baudrate invalido\r\n");
        return;
    }

    if (baud < 1200 || baud > 921600) {
        writeSerialCom("Baudrate fuera de rango\r\n");
        return;
    }

    if (!setUartBaudrate(baud)) {
        writeSerialCom("Error al guardar baudrate\r\n");
        return;
    }
    writeSerialCom("Baudrate guardado\r\n");
    writeSerialCom("Reinicie para aplicar cambios\r\n");
    return;
}

    if(menu->id == 23){

    uint16_t buf_size;
    if (sscanf(data, "%hu", &buf_size) != 1) {
        writeSerialCom("Tamaño de buffer invalido\r\n");
        return;
    }
    if (buf_size < 128 || buf_size > 16384) {
        writeSerialCom("Tamaño de buffer fuera de rango (128-16384)\r\n");
        return;
    }
    setUartBufferSize(buf_size);
    writeSerialCom("Tamaño de buffer guardado\r\n");
    writeSerialCom("Reinicie para aplicar cambios\r\n");
    return;






    
}


}


static void onEnterNode(MenuNode* n) {
    if (!n) return;

    if (nodeRequiresInput(n->id)) {
        aceptandoDatos = false;
        memset(data_buffer, 0, sizeof(data_buffer));
        buffer_index = 0;
        //sendUartDataln("Nodo requiere entrada. Presiona 'ENTER' para comenzar.");


    } 

    // Acciones inmediatas (sin pedir datos) y automaticas en elupdate
    switch (n->id) {
        case 11:  // Entradas analógicas
            printNetworkInfo();
            break;
        case 21:  // Información UART1
            printUartInfo();
            break;
        default:
            break;
    }

    // Nodos que requieren datos: activar captura y mostrar prompt
    if (nodeRequiresInput(n->id)) {
        aceptandoDatos = true;
        memset(data_buffer, 0, sizeof(data_buffer));
        buffer_index = 0;

        switch (n->id) {
            //case 3:  writeSerialComln("Ingrese SSID y presione 'ENTER' para confirmar"); break;
            case 12: writeSerialComln("Ingrese el nuevo SSID y presione 'ENTER' para confirmar"); break;
            case 13: writeSerialComln("Ingrese la nueva contraseña y presione 'ENTER' para confirmar"); break;
            case 14: writeSerialComln("Ingrese el nuevo puerto TCP y presione 'ENTER' para confirmar");break;
            case 22: writeSerialComln("Ingrese el baudrate y presione 'ENTER' para confirmar"); break;
            case 23: writeSerialComln("Ingrese el tamaño del buffer y presione 'ENTER' para confirmar"); break;
            default: break;
        }
    }
}

//Ejecuto acciones periódicas al estar en ciertos nodos
static void onUpdateNode(MenuNode* n) {
    if (!n) return;

    switch (n->id) {
        case 21: 
            updateUartBuffers();
            break;

        default:
            // Otros menús no se refrescan constantemente
            break;
    }
}

static bool nodeRequiresInput(int id) {
    switch (id) {

        case 12:  // Cambiar SSID
        case 13:  // Cambiar Password
        case 14:  // Cambiar Puerto TCP
        case 22:  // Cambiar Baudrate
        case 23:  // Cambiar Tamaño de Buffer UART
        
            return true;
        default:
            return false;
    }
}