#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define LONGMENSAJE 8*1024

void enviameDirectorio(int sockclifd);
void enviameUnArchivo(int sockclifd);
void modificaArchivo(int sockclifd);
void eliminaArchivo(int sockclifd);
void creaArchivo(int sockclifd);
void cierraConexion(int sockclifd);
int menu(void);

bool EstaLleno(sockclifd);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <IP> <Puerto>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockclifd;
    struct sockaddr_in addrcli_in;

    // Crear un socket TCP/IP
    if ((sockclifd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Configurar la direcci�n del cliente
    addrcli_in.sin_family = AF_INET;
    addrcli_in.sin_port = htons(atoi(argv[2]));
    addrcli_in.sin_addr.s_addr = inet_addr(argv[1]);
    memset(addrcli_in.sin_zero, 0, sizeof(addrcli_in.sin_zero));

    // Conectar al servidor
    if (connect(sockclifd, (struct sockaddr *)&addrcli_in, sizeof(addrcli_in)) < 0) {
        perror("Error en connect");
        close(sockclifd);
        exit(EXIT_FAILURE);
    }
    
    if(EstaLleno(sockclifd)){
    	cierraConexion(sockclifd);
    	close(sockclifd);
    	
    	return(0);
    }

    printf("Cliente conectado con el servidor\n");
    int opcion;
    while ((opcion = menu()) != 9) {
        switch (opcion) {
            case 1:
                enviameDirectorio(sockclifd);
                break;
            case 2:
                enviameUnArchivo(sockclifd);
                break;
            case 3:
                modificaArchivo(sockclifd);
                break;
            case 4:
                eliminaArchivo(sockclifd);
                break;
            case 5:
                creaArchivo(sockclifd);
                break;
            default:
                printf("Opci�n no v�lida\n");
                break;
        }
    }
    cierraConexion(sockclifd);
    close(sockclifd);
    return 0;
}

bool EstaLleno(int sockclifd)
{
    char mensaje[LONGMENSAJE];
    int nb = recv(sockclifd, mensaje, LONGMENSAJE, 0);
    if (nb > 0)
    {
        mensaje[nb] = '\0';
        if (strcmp(mensaje, "Servidor lleno, por favor intente m�s tarde.") == 0)
        {
            printf("Error: %s\n", mensaje);
            return true;
        }
        
        printf(mensaje,"\n");
        return false;
        
    }
}

// Solicitar y mostrar el listado de archivos en el directorio del servidor
void enviameDirectorio(int sockclifd) {
    char mensaje[LONGMENSAJE];
    memset(mensaje, 0, sizeof(mensaje));
    strcpy(mensaje, "1");
    send(sockclifd, mensaje, strlen(mensaje), 0);

    printf("Archivos del directorio de trabajo:\n");
    while (1) {
        int nb = recv(sockclifd, mensaje, sizeof(mensaje) - 1, 0);
        if (nb > 0) {
            mensaje[nb] = '\0';
            if (strstr(mensaje, "\nEND\n") != NULL) {
                mensaje[nb - strlen("\nEND\n")] = '\0';
                printf("%s", mensaje);
                break;
            }
            printf("%s", mensaje);
        } else {
            if (nb < 0) perror("Error al recibir el mensaje");
            break;
        }
    }
    printf("\n");
}

// Solicitar y mostrar el contenido de un archivo en el servidor
void enviameUnArchivo(int sockclifd) {
    char nombreArchivo[64];
    memset(nombreArchivo, 0, sizeof(nombreArchivo));
    char mensaje[LONGMENSAJE];
    memset(mensaje, 0, sizeof(mensaje));
    strcpy(mensaje, "2");
    printf("Ingrese el nombre del archivo: ");
    scanf("%63s", nombreArchivo);
    strcat(mensaje, nombreArchivo);
    send(sockclifd, mensaje, strlen(mensaje), 0);

    // Recepci�n del archivo en partes
    printf("Contenido del Archivo:\n");
    while (1) {
        int nb = recv(sockclifd, mensaje, sizeof(mensaje) - 1, 0);
        if (nb > 0) {
            mensaje[nb] = '\0';
            if (strstr(mensaje, "\nEND\n") != NULL) {
                mensaje[nb - strlen("\nEND\n")] = '\0';
                printf("%s", mensaje);
                break;
            }
            printf("%s", mensaje);
        } else {
            if (nb < 0) perror("Error al recibir el mensaje");
            break;
        }
    }
    printf("\n");
}

// Modificar el contenido de un archivo en el servidor
void modificaArchivo(int sockclifd) {
    char nombreArchivo[64];
    char nuevoContenido[LONGMENSAJE];
    memset(nombreArchivo, 0, sizeof(nombreArchivo));
    memset(nuevoContenido, 0, sizeof(nuevoContenido));
    char mensaje[LONGMENSAJE];
    memset(mensaje, 0, sizeof(mensaje));
    strcpy(mensaje, "2");
    printf("Ingrese el nombre del archivo a modificar: ");
    scanf("%63s", nombreArchivo);
    getchar();  // Para limpiar el salto de l�nea del buffer de entrada
    strcat(mensaje, nombreArchivo);
    send(sockclifd, mensaje, strlen(mensaje), 0);

    // Mostrar el contenido actual del archivo
    printf("Contenido actual del archivo:\n");
    while (1) {
        int nb = recv(sockclifd, mensaje, sizeof(mensaje) - 1, 0);
        if (nb > 0) {
            mensaje[nb] = '\0';
            if (strstr(mensaje, "\nEND\n") != NULL) {
                mensaje[nb - strlen("\nEND\n")] = '\0';
                printf("%s", mensaje);
                break;
            }
            printf("%s", mensaje);
        } else {
            if (nb < 0) perror("Error al recibir el mensaje");
            break;
        }
    }
    printf("\nIngrese las modificaciones en el formato 'nombre dorsal', o 'eliminar nombre' para eliminar, o 'agregar nombre dorsal' para agregar:\n");
    fgets(nuevoContenido, LONGMENSAJE, stdin);
    strcpy(mensaje, "3");
    strcat(mensaje, nombreArchivo);
    strcat(mensaje, "\n");
    strcat(mensaje, nuevoContenido);
    send(sockclifd, mensaje, strlen(mensaje), 0);

    // Recepci�n de la confirmaci�n
    printf("Respuesta del servidor:\n");
    while (1) {
        int nb = recv(sockclifd, mensaje, sizeof(mensaje) - 1, 0);
        if (nb > 0) {
            mensaje[nb] = '\0';
            if (strstr(mensaje, "\nEND\n") != NULL) {
                mensaje[nb - strlen("\nEND\n")] = '\0';
                printf("%s\n", mensaje);
                break;
            }
            printf("%s\n", mensaje);
        } else {
            if (nb < 0) perror("Error al recibir el mensaje");
            break;
        }
    }
    printf("\n");
}

// Eliminar un archivo en el servidor
void eliminaArchivo(int sockclifd) {
    char nombreArchivo[64];
    memset(nombreArchivo, 0, sizeof(nombreArchivo));
    char mensaje[LONGMENSAJE];
    memset(mensaje, 0, sizeof(mensaje));
    strcpy(mensaje, "4");
    printf("Ingrese el nombre del archivo a eliminar: ");
    scanf("%63s", nombreArchivo);
    strcat(mensaje, nombreArchivo);
    send(sockclifd, mensaje, strlen(mensaje), 0);

    // Recepci�n de la confirmaci�n
    printf("Respuesta del servidor:\n");
    while (1) {
        int nb = recv(sockclifd, mensaje, sizeof(mensaje) - 1, 0);
        if (nb > 0) {
            mensaje[nb] = '\0';
            if (strstr(mensaje, "\nEND\n") != NULL) {
                mensaje[nb - strlen("\nEND\n")] = '\0';
                printf("%s\n", mensaje);
                break;
            }
            printf("%s\n", mensaje);
        } else {
            if (nb < 0) perror("Error al recibir el mensaje");
            break;
        }
    }
    printf("\n");
}

// Crear un nuevo archivo en el servidor con contenido especificado
void creaArchivo(int sockclifd) {
    char nombreArchivo[64];
    char contenido[LONGMENSAJE];
    memset(nombreArchivo, 0, sizeof(nombreArchivo));
    memset(contenido, 0, sizeof(contenido));
    char mensaje[LONGMENSAJE];
    memset(mensaje, 0, sizeof(mensaje));
    strcpy(mensaje, "5");
    printf("Ingrese el nombre del nuevo archivo: ");
    scanf("%63s", nombreArchivo);
    getchar();  // Para limpiar el salto de l�nea del buffer de entrada
    printf("Ingrese el contenido del nuevo archivo en el formato 'nombre dorsal', una l�nea por camiseta:\n");
    fgets(contenido, LONGMENSAJE, stdin);
    strcat(mensaje, nombreArchivo);
    strcat(mensaje, "\n");
    strcat(mensaje, contenido);
    send(sockclifd, mensaje, strlen(mensaje), 0);

    // Recepci�n de la confirmaci�n
    printf("Respuesta del servidor:\n");
    while (1) {
        int nb = recv(sockclifd, mensaje, sizeof(mensaje) - 1, 0);
        if (nb > 0) {
            mensaje[nb] = '\0';
            if (strstr(mensaje, "\nEND\n") != NULL) {
                mensaje[nb - strlen("\nEND\n")] = '\0';
                printf("%s\n", mensaje);
                break;
            }
            printf("%s\n", mensaje);
        } else {
            if (nb < 0) perror("Error al recibir el mensaje");
            break;
        }
    }
    printf("\n");
}

// Cerrar la conexi�n con el servidor
void cierraConexion(int sockclifd) {
    char mensaje[LONGMENSAJE];
    memset(mensaje, 0, sizeof(mensaje));
    strcpy(mensaje, "9");
    send(sockclifd, mensaje, strlen(mensaje), 0);
    memset(mensaje, 0, sizeof(mensaje));
    int nb = recv(sockclifd, mensaje, sizeof(mensaje) - 1, 0);
    if (nb > 0) {
        mensaje[nb] = '\0';
        printf("Cierre de conexi�n: %s\n", mensaje);
    } else {
        perror("Error al recibir el mensaje");
    }
}

// Mostrar el men� de opciones al usuario
int menu(void) {
    printf("Menu de opciones del Cliente\n");
    printf("1. Enviame el Directorio\n");
    printf("2. Pedir un archivo\n");
    printf("3. Modificar un archivo\n");
    printf("4. Eliminar un archivo\n");
    printf("5. Crear un archivo\n");
    printf("9. Cerrar conexion\n");
    printf("_________________\n");
    printf("Ingrese opci�n: ");
    int opcion;
    scanf("%d", &opcion);
    return opcion;
}
