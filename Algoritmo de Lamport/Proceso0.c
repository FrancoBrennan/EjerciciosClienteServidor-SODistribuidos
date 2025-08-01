#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define ipServidor "192.168.56.106"   // IP del proceso 1

void DameHoraMaquina(char *Hora);

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int puertoServidor = atoi(argv[1]);

    int dccliente;
    struct sockaddr_in remoto;

    // Crear socket
    dccliente = socket(AF_INET, SOCK_STREAM, 0);
    if (dccliente == -1) {
        perror("socket fallo");
        exit(EXIT_FAILURE);
    }

    // Rellenamos la estructura sockaddr_in remoto
    remoto.sin_family = AF_INET; // Dominio de Internet
    remoto.sin_port = htons(puertoServidor); // Puerto de escucha del servidor
    remoto.sin_addr.s_addr = inet_addr(ipServidor); // Dirección IPv4 donde está corriendo el servidor
    memset(remoto.sin_zero, 0, 8); // Completamos la estructura con ceros

    // Establecemos conexión
    if (connect(dccliente, (struct sockaddr *)&remoto, sizeof(struct sockaddr)) == -1) {
        perror("Conexion fallo");
        exit(EXIT_FAILURE);
    }

    //----------------------------
    printf("Conexion establecida con exito...\n");
    //----------------------------

    char bufin[512];
    printf("\nHora de envio del mensaje A: ");
    memset(bufin, 0, 512);
    DameHoraMaquina(bufin);
    
    //sleep(10);
    
    printf("%s\n", bufin);

    if (send(dccliente, bufin, strlen(bufin), 0) == -1) {
        perror("send fallo");
        exit(EXIT_FAILURE);
    }

    close(dccliente);

    return 0;
}

void DameHoraMaquina(char *Hora)
{
    time_t tiempo = time(0);
    struct tm *tlocal = localtime(&tiempo);
    strftime(Hora, 128, "%H:%M:%S", tlocal);
}

