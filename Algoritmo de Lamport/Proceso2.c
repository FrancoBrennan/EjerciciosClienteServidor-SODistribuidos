
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

#define IP "192.168.56.106" // Ubicacion del servidor

void UbicacionDelCliente(struct sockaddr_in);
void RecibeEnviaMensaje(int);
void DameHoraMaquina(char *);

int main(int argc, char *argv[])
{
    socklen_t addrlen;
    int idsocks;
    struct sockaddr_in addr_in, addrcli_in;
    idsocks = socket(AF_INET, SOCK_STREAM, 0);
    printf("idsocks %d\n", idsocks);

    int puerto = atoi(argv[1]);

    // Completar la estructura addr_in
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(puerto);
    addr_in.sin_addr.s_addr = inet_addr(IP);

    memset(addr_in.sin_zero, 0, 8);

    addrlen = sizeof(addr_in);
    printf("bind %d\n", bind(idsocks, (struct sockaddr *)&addr_in, addrlen));
    printf("listen %d\n", listen(idsocks, 5));

    while (1)
    {
        printf("Servidor en puerto %d esperando conexion de cliente... \n", puerto);
        int idsockc = accept(idsocks, (struct sockaddr *)&addrcli_in, &addrlen);

        if (idsockc != -1)
        {
            if (!fork())
            {
                printf("Conexion aceptada desde el cliente \n");

                char horaC[30];
                char horaS[128];
                int defout;
                int nb = read(idsockc, horaC, 30);

                horaC[nb] = '\0';
                printf("Hora de envio del mensaje B: %s\n", horaC);
                
		//sleep(10);
		
                DameHoraMaquina(horaS);
                printf("Hora de recepcion del mensaje B: %s\n", horaS);

                int hrC, hrS, minC, minS, segC, segS;
                hrC = atoi(strtok(horaC, ":"));
                minC = atoi(strtok(NULL, ":"));
                segC = atoi(strtok(NULL, ":"));

                hrS = atoi(strtok(horaS, ":"));
                minS = atoi(strtok(NULL, ":"));
                segS = atoi(strtok(NULL, ":"));

                if (hrC > hrS)
                {
                    hrS = hrC + 1;
                    printf("\nCorregir");
                    printf("\n Nueva hora %d:%d: %d", hrS, minS, segS);
                }
                else if (hrC == hrS)
                {
                    if (minC > minS)
                    {
                        minS = minC + 1;
                        printf("\nCorregir");
                        printf("\n Nueva hora %d:%d: %d", hrS, minS, segS);
                    }
                    else if (minC == minS)
                    {
                        if (segC > segS || segC == segS)
                        {
                            segS = segC + 1;
                            printf("\nCorregir");
                            printf("\n Nueva hora %d:%d: %d", hrS, minS, segS);
                        }
                    }
                }
            }
        }
        else
        {
            printf("Servidor dice conexion rechazada %d \n", idsockc);
        }
    }
}

void UbicacionDelCliente(struct sockaddr_in c_sock)
{
    printf("............c_sock.sin_family %d\n", c_sock.sin_family);
    printf("............c_sock.sin_port %d\n", c_sock.sin_port);
    printf("............c_sock.sin_addr.s_addr %s\n\n", inet_ntoa(c_sock.sin_addr));
}

void DameHoraMaquina(char *hora)
{
    time_t tiempo = time(0);
    struct tm *tlocal = localtime(&tiempo);
    strftime(hora, 128, "%H:%M:%S", tlocal);
}
