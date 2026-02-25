



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













    1-ToDo: Agregar un menu para que se pueda putnear las 2 uarts. La idea es que acitvando una opcion se puede ver que envia
    la UART1 en la UART0(consola). Y tambien poder hablar con la UART1.

    2-Por otro lado ahcer lo mismo pero con tcp, o sea UART0->TCP y TCP-> UART0 para poder saber si estan llegando datos o no

   Para 1- transmitUartTcp agregar un envio de datos una vez se leen por la UART, el falg deberia ser una OR ya que si no se tiene WIFI igual
   es preferible poder debugear(la UART1 deberia poder funcionar tenga wifi o no)

   3- Agregar una forma para que se muestre el mismo menu usando el puerto TCP 4000