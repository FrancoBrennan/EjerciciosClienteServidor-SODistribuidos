#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define TAMPAGINA 10
#define LONGMENSAJE 8*1024

void imprimirMenu() {
    printf("Menú:\n");
    printf("1. Ver contenido del vector\n");
    printf("2. Modificar posición del vector\n");
    printf("3. Liberar posición del vector\n");
    printf("9. Salir\n");
}

int main(int argc, char *argv[]) {
    if(argc != 3){
        printf("La linea de comando es: ./cliente IP PUERTO\n");
        exit(0);
    }
    
    // Definicion de variables
    struct sockaddr_in s_sock; 
    int idsock; 
  
    // Llamada al sistema socket
    idsock = socket(AF_INET, SOCK_STREAM, 0); 
    if(idsock == -1) {
        perror("socket");
        exit(1);
    }
  
    // Completamos la estructura sockaddr_in
    s_sock.sin_family = AF_INET; 
    s_sock.sin_port = htons(atoi(argv[2])); 
    s_sock.sin_addr.s_addr = inet_addr(argv[1]); 
    memset(s_sock.sin_zero, 0, 8); 
  
    // Conectar al servidor
    if (connect(idsock, (struct sockaddr *)&s_sock, sizeof(s_sock)) == -1) {
        perror("connect");
        exit(1);
    }
  
    char mensaje[LONGMENSAJE];
    char opcion;
    while (1) {
        imprimirMenu();
        printf("Seleccione una opción: ");
        scanf(" %c", &opcion);
        
        if (opcion == '1') {
            // Ver contenido del vector
            int posicion;
            printf("Ingrese posición (0-31): ");
            scanf("%d", &posicion);
            if(posicion >= TAMPAGINA){
            	printf("Posición inválida\n");
            	continue;
            }
            sprintf(mensaje, "1%d", posicion);
            send(idsock, mensaje, strlen(mensaje), 0);
            int nb = recv(idsock, mensaje, LONGMENSAJE, 0);
            if (nb > 0) {
                mensaje[nb] = '\0';
                printf("Respuesta del servidor: %s\n", mensaje);
            } else {
                perror("recv");
            }
        } else if (opcion == '2') {
            // Modificar posición del vector
            int posicion;
            char nuevoValor;
            printf("Ingrese posición (0-9): ");
            scanf("%d", &posicion);
            if(posicion >= TAMPAGINA){
            	printf("Posición inválida\n");
            	continue;
            }
            printf("Ingrese nuevo valor: ");
            scanf(" %c", &nuevoValor);
            sprintf(mensaje, "2%d%c", posicion, nuevoValor);
            send(idsock, mensaje, strlen(mensaje), 0);
            int nb = recv(idsock, mensaje, LONGMENSAJE, 0);
            if (nb > 0) {
                mensaje[nb] = '\0';
                printf("Respuesta del servidor: %s\n", mensaje);
            } else {
                perror("recv");
            }
        } else if (opcion == '3') {
            // Liberar posición del vector
            int posicion;
            printf("Ingrese posición a liberar (0-9): ");
            scanf("%d", &posicion);
            if(posicion >= TAMPAGINA){
            	printf("Posición inválida\n");
            	continue;
            }
            sprintf(mensaje, "3%d", posicion);
            send(idsock, mensaje, strlen(mensaje), 0);
            int nb = recv(idsock, mensaje, LONGMENSAJE, 0);
            if (nb > 0) {
                mensaje[nb] = '\0';
                printf("Respuesta del servidor: %s\n", mensaje);
            } else {
                perror("recv");
            }
        } else if (opcion == '9') {
            // Salir
            mensaje[0] = '9';
            mensaje[1] = '\0';
            send(idsock, mensaje, strlen(mensaje), 0);
            break;
        } else {
            printf("Opción inválida\n");
            continue;
        }
    }

    close(idsock);
    return 0;
}

