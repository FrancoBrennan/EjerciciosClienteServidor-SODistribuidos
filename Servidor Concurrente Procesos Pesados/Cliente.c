/*
 Cliente en Linux de mini Telnet de un servidor de comandos Debian
 Hay que agregarle el ingreso de un usuario y contraseña
 para que sea validado por el servidor.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // Cabecera para los sockets en Linux
#include <netdb.h> // Cabecera para gethostbyaddr
#include <signal.h>

#define ipServidor "127.0.0.1"
#define puertoServidor 6666

void UbicacionDelServidor(struct in_addr);
void handler_sigint(int);

int dccliente;

int main ()
{
    signal(SIGINT, handler_sigint);
    //int dccliente; // Contiene el descriptor de socket cliente
    struct sockaddr_in remoto; // Contiene los datos de conexion del equipo remoto

    // Obtiene el identificador del socket
    dccliente = socket(AF_INET, SOCK_STREAM, 0);
    if (dccliente == -1)
    {
        perror("socket fallo");
        exit(1);
    }

    // Rellenamos la estructura sockaddr_in remoto
    remoto.sin_family = AF_INET; // Dominio de Internet
    remoto.sin_port = htons(puertoServidor); // puerto de escucha del servidor
    remoto.sin_addr.s_addr = inet_addr(ipServidor); // dirección IPv4 donde está corriendo el servidor
    memset(remoto.sin_zero, 0, sizeof(remoto.sin_zero)); // Completamos la estructura con ceros

    // Establecemos conexión
    if (connect(dccliente, (struct sockaddr*)&remoto, sizeof(remoto)) == -1)
    {
        perror("Conexion fallo");
        close(dccliente);
        exit(1);
    }

    printf("Conexion establecida con exito... Datos del Servidor...\n");
    UbicacionDelServidor(remoto.sin_addr);

    char bufin[512];
    printf("\nIngrese comando Linux: ");
    while (fgets(bufin, sizeof(bufin), stdin))
    {
        bufin[strcspn(bufin, "\n")] = '\0'; // Elimina el salto de línea

        if (strcmp(bufin, "exit") == 0)
            break;

        if (send(dccliente, bufin, strlen(bufin), 0) == -1)
        {
            perror("send fallo");
            close(dccliente);
            exit(1);
        }

        char buf[4096];
        int nb;
        if ((nb = recv(dccliente, buf, sizeof(buf) - 1, 0)) == -1)
        {
            perror("recv fallo");
            close(dccliente);
            exit(1);
        }

        buf[nb] = '\0';
        printf("%s\n", buf);
        printf("\nIngrese comando Linux: ");
    }

    if (send(dccliente, "exit", strlen("exit"), 0) == -1)
    {
        perror("send fallo");
        close(dccliente);
        exit(1);
    }

    printf("Conexion terminada\n");
    close(dccliente);
    return 0;
}

void UbicacionDelServidor(struct in_addr addr)
{
    struct hostent *remoteHost;
    char **pAlias;
    remoteHost = gethostbyaddr(&addr, sizeof(addr), AF_INET);
    if (remoteHost == NULL)
    {
        herror("Fallo gethostbyaddr");
    }
    else
    {
        printf("Nombre Servidor: %s\n", remoteHost->h_name);
        for (pAlias = remoteHost->h_aliases; *pAlias != NULL; pAlias++)
        {
            printf("Nombre Alternativo: %s\n", *pAlias);
        }
        printf("Tipo Address: ");
        switch (remoteHost->h_addrtype)
        {
        case AF_INET:
            printf("AF_INET\n");
            break;
        case AF_INET6:
            printf("AF_INET6\n");
            break;
        default:
            printf("%d\n", remoteHost->h_addrtype);
            break;
        }
        printf("Bytes Address: %d\n", remoteHost->h_length);
        if (remoteHost->h_addrtype == AF_INET)
        {
            int i = 0;
            while (remoteHost->h_addr_list[i] != NULL)
            {
                addr.s_addr = *(uint32_t *)remoteHost->h_addr_list[i++];
                printf("IPv4 Address #%d: %s\n", i, inet_ntoa(addr));
            }
        }
        else if (remoteHost->h_addrtype == AF_INET6)
        {
            printf("IPv6 address\n");
        }
    }
}

void handler_sigint(int s) {
	send(dccliente, "exit", strlen("exit"), 0);
	exit(0);
}

