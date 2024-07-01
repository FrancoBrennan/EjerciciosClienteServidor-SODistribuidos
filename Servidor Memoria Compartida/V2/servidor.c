#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>

#define TAMPAGINA 10
#define LONGMENSAJE 8*1024

void UbicacionDelCliente(struct sockaddr_in); 
void *Atender(void *sockclifd);

char *vecPuntPag;
sem_t semaforos[TAMPAGINA]; // Array de semáforos

int main(int argc, char *argv[]) 
{
    if(argc != 3){
        printf("La linea de comando es: ./servCon IP PUERTO\n");
        exit(0);
    }
    
    // Definicion de variables
    struct sockaddr_in s_sock, c_sock; 
    int idsocks, idsockc; 
    int lensock = sizeof(struct sockaddr_in); 
    pthread_t thread_id;
  
    // Crear memoria compartida
    int shmid = shmget(IPC_PRIVATE, TAMPAGINA, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }
    vecPuntPag = (char *)shmat(shmid, NULL, 0);
    if (vecPuntPag == (char *)-1) {
        perror("shmat");
        exit(1);
    }
    memset(vecPuntPag, '-', TAMPAGINA); // Inicializar memoria compartida con '-'
  
    // Inicializa los semáforos
    for (int i = 0; i < TAMPAGINA; i++) {
        if (sem_init(&semaforos[i], 0, 1) == -1) {
            perror("sem_init");
            exit(1);
        }
    }

    // Llamada al sistema socket
    idsocks = socket(AF_INET, SOCK_STREAM, 0); 
    printf("idsocks %d\n", idsocks);
  
    // Completamos la estructura sockaddr_in
    s_sock.sin_family = AF_INET; 
    s_sock.sin_port = htons(atoi(argv[2])); 
    s_sock.sin_addr.s_addr = inet_addr(argv[1]); 
    memset(s_sock.sin_zero, 0, 8); 
  
    // Llamada al sistema bind
    printf("bind %d\n", bind(idsocks, (struct sockaddr *) &s_sock, lensock));
    
    // Llamada al sistema listen
    printf("listen %d\n", listen(idsocks, 5)); 
  
    while(1) 
    { 
        printf("esperando conexion\n"); 
        idsockc = accept(idsocks, (struct sockaddr *)&c_sock, &lensock); 
        if(idsockc != -1) 
        { 
            /* Ubicacion del Cliente */ 
            printf("conexion aceptada desde el cliente\n"); 
            UbicacionDelCliente(c_sock); 
            /*--------------------------------------------------*/ 
            int *pclient = malloc(sizeof(int));
            *pclient = idsockc;
            if (pthread_create(&thread_id, NULL, Atender, pclient) != 0) {
                perror("pthread_create");
            }
        } 
        else
        {
            printf("conexion rechazada %d \n", idsockc); 
        }
    }
    
    // Destruye los semáforos
    for (int i = 0; i < TAMPAGINA; i++) {
        sem_destroy(&semaforos[i]);
    }

    exit(0);
} 

void *Atender(void *sockclifd) {
    int sockcli = *((int *) sockclifd);
    free(sockclifd); // Liberar la memoria asignada para el cliente
    char mensaje[LONGMENSAJE];
    char opcion;

    do {
        memset(mensaje, '\0', LONGMENSAJE);
        int nb = recv(sockcli, mensaje, LONGMENSAJE, 0);
        if (nb <= 0) {
            perror("recv");
            break;
        }
        mensaje[nb] = '\0';
        opcion = mensaje[0];
        printf("cliente %d dice -> %s\n", sockcli, mensaje);

        if (opcion == '1') { // Cliente pide el contenido de una posición del vector
            int posicion = mensaje[1] - '0';
            if (posicion >= 0 && posicion < TAMPAGINA) {
                if (sem_trywait(&semaforos[posicion]) == 0) { // Intentar entrar a la sección crítica sin bloqueo
                    char contenido = vecPuntPag[posicion];
                    snprintf(mensaje, LONGMENSAJE, "Contenido en posición %d: %c\n", posicion, contenido);
                    sem_post(&semaforos[posicion]); // Salir de la sección crítica
                    
                    printf("Contenido del vector en memoria compartida:\n");
                	  for (int i = 0; i < TAMPAGINA; i++) {
                    		printf("vecPuntPag[%d] = %c\n", i, vecPuntPag[i]);
                	  }
                } else {
                    strcpy(mensaje, "El semaforo esta ocupado, intentelo mas tarde\n");
                }
            } else {
                strcpy(mensaje, "Posicion invalida\n");
            }
        } else if (opcion == '2') { // Cliente modifica una posición del vector
            int posicion = mensaje[1] - '0';
            char nuevoValor = mensaje[2];
            if (posicion >= 0 && posicion < TAMPAGINA) {
                if (sem_trywait(&semaforos[posicion]) == 0) { // Bloquear el semáforo
                    vecPuntPag[posicion] = nuevoValor;
                    snprintf(mensaje, LONGMENSAJE, "Posicion %d bloqueada para modificar. Ingrese '3' y '%d' para liberar.\n", posicion, posicion);
                    
                    printf("Contenido del vector en memoria compartida:\n");
                	  for (int i = 0; i < TAMPAGINA; i++) {
                    		printf("vecPuntPag[%d] = %c\n", i, vecPuntPag[i]);
                	  }
                } else {
                    strcpy(mensaje, "El semaforo esta ocupado, intentelo mas tarde\n");
                }
            } else {
                strcpy(mensaje, "Posicion invalida\n");
            }
        } else if (opcion == '3') { // Cliente libera una posición del vector
            int posicion = mensaje[1] - '0';
            if (posicion >= 0 && posicion < TAMPAGINA) {
                sem_post(&semaforos[posicion]); // Liberar el semáforo
                snprintf(mensaje, LONGMENSAJE, "Posicion %d liberada\n", posicion);
            } else {
                strcpy(mensaje, "Posicion invalida\n");
            }
        }

        send(sockcli, mensaje, strlen(mensaje), 0);
    } while (opcion != '9');

    close(sockcli);
    pthread_exit(NULL);
}

void UbicacionDelCliente(struct sockaddr_in c_sock) 
{ 
    printf("............c_sock.sin_family %d\n", c_sock.sin_family); 
    printf("............c_sock.sin_port %d\n", ntohs(c_sock.sin_port)); 
    printf("............c_sock.sin_addr.s_addr %s\n\n", inet_ntoa(c_sock.sin_addr)); 
}

