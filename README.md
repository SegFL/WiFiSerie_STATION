



Caso de uso:

Al encender la ESP32 se intenta conectar a una red WiFi con los ssid y contraseña definidos. 
Si se realiza una conexion se imprime por pantalla la IP que se le asigno y la ip del router.
Luego crea un servidor TCP con la IP que le asignaron y el puerto 5000, se queda esperando a 
que un cliente se conecte para ahcer un loopback.





Conexion TCP

    socket() : Crea el socker de la conexion
    bind() : Inicia el socket
    listen(): se qeda esperando a que alguien se conecte
    while(1){
        accept() : Acepta un cliente
        while(1){
            recv() :recive datos 
            send() : reenvia datos
        }
    }