#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define IP_SERVIDOR "127.0.0.1"

void DameHoraMaquina(char *);

int main(int argc, char *argv[])
{
    socklen_t addrlen;
    int idsocks;
    struct sockaddr_in addr_in;
    idsocks = socket(AF_INET, SOCK_STREAM, 0);
    printf("idsocks %d\n", idsocks);

    int puertoServidor = atoi(argv[1]);

    addr_in.sin_family = AF_INET;
    addr_in.sin_addr.s_addr = inet_addr(IP_SERVIDOR);
    addr_in.sin_port = htons(puertoServidor);
    memset(addr_in.sin_zero, 0, sizeof(addr_in.sin_zero));

    if (idsocks > 0)
    {
        printf("Conectando con el servidor %d \n", idsocks);

        int con = connect(idsocks, (struct sockaddr *)&addr_in, sizeof(addr_in));

        if (con < 0)
        {
            printf("Connection failed \n");
        }

        printf("Cliente conectado con el servidor %d \n", idsocks);

        // Consultar hora. Acá también le mandamos la hora del cliente, pero no es necesaria para este algoritmo.
        char buffc[512];
        memset(buffc, 0, 512);

        DameHoraMaquina(buffc);

        printf("Enviando solicitud de hora al servidor... \n");
        int s = send(idsocks, buffc, strlen(buffc), 0);

        if (s == -1)
        {
            printf("No se pudo enviar la respuesta al cliente");
        }

        char buffs[512];
        memset(buffs, 0, 512);

        int nb = recv(idsocks, buffs, sizeof(buffs) - 1, 0);
        buffs[nb] = '\0';
        printf("Respuesta del servidor: %s\n", buffs);

        close(idsocks);
    }
    else
    {
        printf("Conexion rechazada - isocks <= 0 : %d \n", idsocks);
        exit(0);
    }
}

void DameHoraMaquina(char *buff)
{
    time_t tiempo = time(0);
    struct tm *tlocal = localtime(&tiempo);
    strftime(buff, sizeof(buff), "%H:%M", tlocal);
    printf("DameHoraMaquina Cliente: %s\n", buff);
}
