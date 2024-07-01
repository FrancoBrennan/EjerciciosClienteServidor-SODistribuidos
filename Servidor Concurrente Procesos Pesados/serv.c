/*  
    Mini servidor concurrente comandos Debian  
    con procesos pesados
*/ 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <sys/types.h> 
#include <string.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <netdb.h> 
#include <signal.h>
#include <sys/wait.h>

#define IP "127.0.0.1" // Ubicación del servidor mini telnet 
#define PUERTO 6666 

void UbicacionDelCliente(struct sockaddr_in); 
void RecibeEnviaComandos(int);
void mataZombies_Handler_De_SIGCHLD(int); 

int main() 
{ 
    struct sockaddr_in s_sock, c_sock; 
    int idsocks, idsockc; 
    int lensock = sizeof(struct sockaddr_in); 
    idsocks = socket(AF_INET, SOCK_STREAM, 0); 
    printf("idsocks %d\n", idsocks); 
  
    s_sock.sin_family      = AF_INET; 
    s_sock.sin_port        = htons(PUERTO); 
    s_sock.sin_addr.s_addr = inet_addr(IP); 
    memset(s_sock.sin_zero, 0, 8); 
  
    printf("bind %d\n", bind(idsocks, (struct sockaddr *)&s_sock, lensock)); 
    printf("listen %d\n", listen(idsocks, 5)); 
    signal(SIGCHLD, mataZombies_Handler_De_SIGCHLD);
  
    while(1) 
    { 
        printf("esperando conexion\n"); 
        idsockc = accept(idsocks, (struct sockaddr *)&c_sock, &lensock); 
        if(idsockc != -1) 
        { 
            pid_t pid = fork();
            
            if(pid == 0)
            {
                signal(SIGCHLD, SIG_DFL);
                printf("Conexion aceptada desde el cliente \n");
                UbicacionDelCliente(c_sock);
                
                RecibeEnviaComandos(idsockc);
                close(idsockc);
                exit(0);
            }
            else if(pid == -1) 
            {
                printf("fallo fork()\n");
            }
        } 
        else  
        { 
            printf("conexion rechazada %d \n", idsockc); 
        } 
    } 
} 

void RecibeEnviaComandos(int idsockc) 
{ 
    char comando[30];
    char salida[8192];
    int p[2];
    int nb; 
    int defout; 
    nb = recv(idsockc, comando, 30, 0); 
    while(strncmp(comando, "exit", 4)) 
    { 
        pipe(p);
        comando[nb] = '\0'; 
        printf(".......recibido del cliente %d : %s\n", idsockc, comando); 
        defout = dup(1); 
        dup2(p[1], 1); 
        system(comando); 
        dup2(defout, 1); 
        close(defout);
        close(p[1]);
        memset(salida, '\0', 8192); 
        nb = read(p[0], salida, 8192);
        salida[nb] = '\0';
        close(p[0]);
        send(idsockc, salida, strlen(salida), 0);
        nb = recv(idsockc, comando, 30, 0);
    } 
}

void UbicacionDelCliente(struct sockaddr_in c_sock) 
{ 
    printf("............c_sock.sin_family %d\n", c_sock.sin_family); 
    printf("............c_sock.sin_port %d\n", ntohs(c_sock.sin_port)); 
    printf("............c_sock.sin_addr.s_addr %s\n\n", inet_ntoa(c_sock.sin_addr)); 
} 

void mataZombies_Handler_De_SIGCHLD(int nrosig) 
{
    printf("Entre a la función asociada a SIGCHLD %d\n", nrosig);
    while(waitpid(-1, NULL, WNOHANG) > 0);
}
