#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>

#define IP "127.0.0.1" // Ubicacion del servidor

void *atender_cliente(void *);
void DameHoraMaquina(char *);

int main(int argc, char *argv[])
{
    socklen_t addrlen;
    int idsocks;
    struct sockaddr_in addr_in, addrcli_in;

    idsocks = socket(AF_INET, SOCK_STREAM, 0);

    printf("idsocks %d\n", idsocks);

    int puerto = atoi(argv[1]);
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(puerto);
    addr_in.sin_addr.s_addr = inet_addr(IP);
    memset(addr_in.sin_zero, 0, sizeof(addr_in.sin_zero));

    addrlen = sizeof(addr_in);
    printf("bind %d\n", bind(idsocks, (struct sockaddr *)&addr_in, addrlen));
    printf("listen %d\n", listen(idsocks, 5));

    printf("Esperando conexiones... \n");

    while (1)
    {
        printf("Servidor en puerto %d esperando conexion de cliente... \n", puerto);

        int *idsockc = malloc(sizeof(int));
        *idsockc = accept(idsocks, (struct sockaddr *)&addrcli_in, &addrlen);

        if (*idsockc == -1)
        {
            printf("Conexion rechazada - idsockc <= 0 : %d \n", *idsockc);
            free(idsockc);
            continue;
        }

        printf("Conexion aceptada desde el cliente %d \n", *idsockc);

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, atender_cliente, idsockc) != 0)
        {
            printf("Error al crear el hilo\n");
            close(*idsockc);
            free(idsockc);
        }
        else
        {
            pthread_detach(thread_id);
        }
    }

    close(idsocks);
}

void *atender_cliente(void *idsockcli)
{
    int idsockc = *(int *)idsockcli;
    free(idsockcli);

    char buffc[512];
    memset(buffc, 0, 512);

    const char *msg = "Por favor, envi­e la hora de su maquina en formato HH:MM";
    send(idsockc, msg, strlen(msg), 0);

    int nb = recv(idsockc, buffc, sizeof(buffc) - 1, 0);
    if (nb > 0)
    {
        buffc[nb] = '\0';
        printf("Hora recibida desde el cliente: %s \n", buffc);

        char buffs[512];
        memset(buffs, 0, 512);
        DameHoraMaquina(buffs);

        struct tm client_tm, server_tm;
        memset(&client_tm, 0, sizeof(struct tm));
        memset(&server_tm, 0, sizeof(struct tm));

        strptime(buffc, "%H:%M", &client_tm);
        strptime(buffs, "%H:%M", &server_tm);

        time_t client_time = mktime(&client_tm);
        time_t server_time = mktime(&server_tm);

        double difference = difftime(server_time, client_time);
        char diff_str[64];

        if (difference > 0)
        {
            snprintf(diff_str, sizeof(diff_str), "+%.0f segundos", difference);
        }
        else
        {
            snprintf(diff_str, sizeof(diff_str), "%.0f segundos", difference);
        }

        printf("Diferencia calculada: %s \n", diff_str);

        // Enviar la diferencia al cliente
        send(idsockc, diff_str, strlen(diff_str), 0);
    }

    close(idsockc);
    pthread_exit(NULL);
}

void DameHoraMaquina(char *buff)
{
    time_t tiempo = time(0);
    struct tm *tlocal = localtime(&tiempo);
    strftime(buff, sizeof(buff), "%H:%M", tlocal);
    printf("DameHoraMaquina Servidor: %s\n", buff);
}
