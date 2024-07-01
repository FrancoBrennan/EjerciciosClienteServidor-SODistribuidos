#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define IP "127.0.0.1"
#define PUERTO 8888
#define TAMPAGINA 32
#define LONGMENSAJE 8 * 1024
#define MAX_PAG 10

typedef struct
{
    char contenido[TAMPAGINA];
    sem_t semaforo;
} Pagina;

Pagina paginas[MAX_PAG];

void *atender_cliente(void *);
void init_memoria_semaforos();
void imprime_info_paginas();

int main(int argc, char *argv[])
{
    socklen_t addrlen;
    int idsocks, idsockc;
    struct sockaddr_in addr_in, addrcli_in;
    pthread_t thread_id;

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

    init_memoria_semaforos();

    while (1)
    {
        printf("Servidor en puerto %d esperando conexion de cliente... \n", puerto);

        imprime_info_paginas();

        idsockc = accept(idsocks, (struct sockaddr *)&addrcli_in, &addrlen);
        if (idsockc == -1)
        {
            printf("Conexion rechazada - isocks <= 0 : %d \n", idsockc);
        }

        printf("Conexion aceptada desde el cliente %d \n", idsockc);

        int *arg = malloc(sizeof(*arg));
        *arg = idsockc;
        if (pthread_create(&thread_id, NULL, atender_cliente, arg) != 0)
        {
            perror("pthread_create");
        }
        else
        {
            pthread_detach(thread_id);
        }
    }

    exit(0);
}

void init_memoria_semaforos()
{
    for (int i = 0; i < MAX_PAG; i++)
    {
        memset(paginas[i].contenido, '-', TAMPAGINA);
        sem_init(&paginas[i].semaforo, 0, 1);
    }
}

void *atender_cliente(void *idsockcli)
{
    int idsockc = *((int *)idsockcli);
    free(idsockcli);

    char buff[LONGMENSAJE];
    char opt;

    while (1)
    {
        memset(buff, '\0', LONGMENSAJE);
        int nb = recv(idsockc, buff, LONGMENSAJE, 0);
        if (nb <= 0)
        {
            perror("recv");
            break;
        }
        buff[nb] = '\0';
        opt = buff[0];

        printf("cliente %d dice -> %s\n", idsockc, buff);

        if (opt == '1')
        {
            int pagina = buff[1] - '0';
            if (pagina >= 0 && pagina < MAX_PAG)
            {
                if (sem_trywait(&paginas[pagina].semaforo) == 0)
                {
                    strncpy(buff, paginas[pagina].contenido, TAMPAGINA);
                    sem_post(&paginas[pagina].semaforo);
                }
                else
                {
                    strcpy(buff, "Pagina en uso, intentelo mas tarde\n");
                }
            }
            else
            {
                strcpy(buff, "Pagina invalida\n");
            }

            imprime_info_paginas();
        }
        else if (opt == '2')
        {
            int pagina = buff[1] - '0';
            if (pagina >= 0 && pagina < MAX_PAG)
            {
                if (sem_trywait(&paginas[pagina].semaforo) == 0)
                {
                    buff[nb] = '\0';
                    strcpy(buff, "Pagina disponible para uso\n");
                    send(idsockc, buff, strlen(buff), 0);

                    nb = recv(idsockc, buff, LONGMENSAJE, 0);
                    if (nb > 0)
                    {
                        buff[nb] = '\0';
                        printf("Cliente %d dice -> %s\n", idsockc, buff);
                        strncpy(paginas[pagina].contenido, buff, TAMPAGINA);

                        sem_post(&paginas[pagina].semaforo);

                        strcpy(buff, "Actualizacion exitosa! \n");
                        send(idsockc, buff, strlen(buff), 0);

                        imprime_info_paginas();
                    }
                    else
                    {
                        perror("recv");
                        strcpy(buff, "Error al recibir contenido\n");
                    }
                }
                else
                {
                    strcpy(buff, "Pagina en uso, intentelo mas tarde\n");
                }
            }
            else
            {
                strcpy(buff, "Pagina invalida \n");
            }
        }
        else if (opt == '9')
        {
            strcpy(buff, "Desconectando\n");
            send(idsockc, buff, strlen(buff), 0);
            break;
        }
        else
        {
            strcpy(buff, "Opcion invalida\n");
        }

        send(idsockc, buff, strlen(buff), 0);
    }

    close(idsockc);
    pthread_exit(NULL);
}

void imprime_info_paginas()
{
    printf("Contenido de todas las paginas:\n");
    for (int i = 0; i < MAX_PAG; i++)
    {
        printf("Pagina %d: %.*s\n", i, TAMPAGINA, paginas[i].contenido);
    }
}
