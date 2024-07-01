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
void AjustarDiferencia(char *, char *);

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
            printf("Fallo con la conexion \n");
            close(idsocks);
            exit(0);
        }

        printf("Cliente conectado con el servidor %d \n", idsocks);

        char buffs[512];
        memset(buffs, 0, 512);

        int nb = recv(idsocks, buffs, sizeof(buffs) - 1, 0);
        if (nb > 0)
        {
            buffs[nb] = '\0';
            printf("Mensaje del servidor: %s\n", buffs);

            char buffc[512];
            memset(buffc, 0, 512);

            DameHoraMaquina(buffc);

            printf("Enviando hora de la maquina al servidor... \n");

            int s = send(idsocks, buffc, strlen(buffc), 0);
            if (s == -1)
            {
                printf("No se pudo enviar la hora al servidor\n");
                close(idsocks);
                exit(0);
            }

            char buffs2[512];
            memset(buffs2, 0, 512);

            nb = recv(idsocks, buffs2, sizeof(buffs2) - 1, 0);
            if (nb > 0)
            {
                buffs2[nb] = '\0';
                printf("Respuesta del servidor: %s\n", buffs2);
                
                AjustarDiferencia(buffc, buffs2);
            }
            
            
        }

        close(idsocks);
    }
    else
    {
        printf("Conexion rechazada - idsocks <= 0 : %d \n", idsocks);
        exit(0);
    }
}

void AjustarDiferencia(char *buffc, char *buffs)
{
    sleep(10);
    time_t tiempo = time(0);
    tiempo = tiempo + (5 * 60);
    struct tm *tlocal = localtime(&tiempo);
    strftime(buff, sizeof(buff), "%H:%M", tlocal);
    printf("DameHoraMaquina Cliente: %s\n", buff);
}
