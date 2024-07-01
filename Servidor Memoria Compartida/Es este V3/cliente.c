#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define IP_SERVIDOR "127.0.0.1"
#define TAMPAGINA 32
#define LONGMENSAJE 8 * 1024

void imprimirMenu()
{
    printf("Menu:\n");
    printf("1. Ver contenido de la pagina\n");
    printf("2. Modificar contenido de la pagina\n");
    printf("9. Salir\n");
}

int main(int argc, char *argv[])
{
    socklen_t addrlen;
    int idsocks;
    struct sockaddr_in addr_in;

    idsocks = socket(AF_INET, SOCK_STREAM, 0);

    printf("idsocks %d\n", idsocks);

    int puerto = atoi(argv[1]);

    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(puerto);
    addr_in.sin_addr.s_addr = inet_addr(IP_SERVIDOR);
    memset(addr_in.sin_zero, 0, sizeof(addr_in.sin_zero));

    if (connect(idsocks, (struct sockaddr *)&addr_in, sizeof(addr_in)) == -1)
    {
        perror("Error al conectar con el servidor");
        close(idsocks);
        exit(1);
    }

    printf("Conexion establecida con exito...\n");

    char buff[LONGMENSAJE];
    char opt;

    while (1)
    {
        imprimirMenu();

        printf("Seleccione una opcion: ");
        scanf(" %c", &opt);
        getchar();

        if (opt == '1')
        {
            int pag;

            printf("Ingrese numero de pagina (0-9): ");

            if (scanf("%d", &pag) != 1 || pag < 0 || pag >= 10)
            {
                printf("Numero de pagina invalido\n");
                getchar();

                continue;
            }
            snprintf(buff, LONGMENSAJE, "1%d", pag);
            if (send(idsocks, buff, strlen(buff), 0) == -1)
            {
                perror("Error al enviar mensaje");

                continue;
            }
            int nb = recv(idsocks, buff, LONGMENSAJE, 0);
            if (nb > 0)
            {
                buff[nb] = '\0';

                printf("Contenido de la pagina %d: %s\n", pag, buff);
            }
            else
            {
                perror("Error al recibir mensaje");
            }
        }
        else if (opt == '2')
        {
            int pag;
            char nuevoContenido[TAMPAGINA];
            printf("Ingrese numero de pagina (0-9): ");
            if (scanf("%d", &pag) != 1 || pag < 0 || pag >= 10)
            {
                printf("Numero de pagina invalido\n");
                getchar(); // Consumir el carÃƒÂ¡cter de nueva lÃƒÂ­nea en caso de error de entrada

                continue;
            }
            getchar(); // Para consumir el carÃƒÂ¡cter de nueva lÃƒÂ­nea dejado por scanf

            // Informar al servidor que la pÃƒÂ¡gina estÃƒÂ¡ bloqueada
            snprintf(buff, LONGMENSAJE, "2%d", pag);
            if (send(idsocks, buff, strlen(buff), 0) == -1)
            {
                perror("Error al enviar mensaje");

                continue;
            }

            int nb = recv(idsocks, buff, LONGMENSAJE, 0);
            if (nb > 0)
            {
                buff[nb] = '\0';
                printf("Respuesta del servidor: %s\n", buff);

                if (strcmp(buff, "Pagina invalida\n") == 0 || strcmp(buff, "Pagina en uso, intentelo mas tarde\n") == 0)
                    continue;
            }
            else
            {
                perror("Error al recibir mensaje");

                continue;
            }

            printf("Ingrese nuevo contenido: ");
            fgets(nuevoContenido, TAMPAGINA, stdin);
            nuevoContenido[strcspn(nuevoContenido, "\n")] = '\0';

            if (send(idsocks, nuevoContenido, strlen(nuevoContenido), 0) == -1)
            {
                perror("Error al enviar nuevo contenido");

                continue;
            }

            nb = recv(idsocks, buff, LONGMENSAJE, 0);
            if (nb > 0)
            {
                buff[nb] = '\0';
                printf("Respuesta del servidor: %s\n", buff);
            }
            else
            {
                perror("Error al recibir respuesta del servidor");
            }
        }
        else if (opt == '9')
        {
            buff[0] = '9';
            buff[1] = '\0';
            if (send(idsocks, buff, strlen(buff), 0) == -1)
            {
                perror("Error al enviar mensaje de salida");
            }
            break;
        }
        else
        {
            printf("Opcion invalida\n");
            continue;
        }
    }

    printf("Conexion terminada\n");
    close(idsocks);

    return 0;
}
