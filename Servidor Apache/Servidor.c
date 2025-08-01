#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define IP "10.180.122.249"
#define PUERTO 80

int main(int argc, char argv[]) {
    char file_name[256] = "Franco/index.html";
    char buf[8192];
    char message[512]; // Incrementé el tamaño del mensaje
    int sd;
    struct sockaddr_in pin;
    pin.sin_family = AF_INET;
    pin.sin_addr.s_addr = inet_addr(IP);
    pin.sin_port = htons(PUERTO);
    bzero(&pin.sin_zero, sizeof(pin.sin_zero));
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Error al abrir el socket\n");
        exit(1);
    }
    if (connect(sd, (void *)&pin, sizeof(pin)) == -1) {
        printf("Error al conectar el socket\n");
        exit(1);
    }
    printf("Conexión establecida\n");
    sprintf(message, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", file_name, IP);
    if (send(sd, message, strlen(message), 0) == -1) {
        printf("Error al enviar\n");
        exit(1);
    }
    printf("Mensaje enviado al servidor Apache:\n%s\n", message);
    if (recv(sd, buf, sizeof(buf), 0) == -1) {
        printf("Error al recibir la respuesta\n");
        exit(1);
    }
    printf("Respuesta del servidor:\n%s\n", buf);
    close(sd);
    return EXIT_SUCCESS;
}
