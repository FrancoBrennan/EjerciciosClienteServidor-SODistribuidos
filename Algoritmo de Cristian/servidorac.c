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
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>

#define IP "127.0.0.1" // Ubicacion del servidor

void MataZombie(int nroSig) {
 while(waitpid(-1, NULL, WNOHANG) > 0);
}
void ParsearHora(char *, char *);
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
    memset(addr_in.sin_zero, 0, sizeof(addr_in.sin_zero));

    addrlen = sizeof(addr_in);
    printf("bind %d\n", bind(idsocks, (struct sockaddr *)&addr_in, addrlen));
    printf("listen %d\n", listen(idsocks, 5));

    signal(SIGCHLD, MataZombie);
    
    printf("Esperando conexiones... \n", puerto);
    
    while (1)
    {
        printf("Servidor en puerto %d esperando conexion de cliente... \n", puerto);
        int idsockc = accept(idsocks, (struct sockaddr *)&addrcli_in, &addrlen);

	if(idsockc == -1) {
		printf("Conexion rechazada - isocks <= 0 : %d \n", idsockc);
	}
	
	printf("Conexion aceptada desde el cliente %d \n", idsockc);

	pid_t pid = fork();
	if(pid == 0) {
		char buffc[512];
		memset(buffc, 0, 512);
		int nb = recv(idsockc, buffc, sizeof(buffc) - 1, 0);
		buffc[nb] = '\0';
		printf("Recibido desde el cliente: %s \n", buffc);
		
		char buffs[512];
		memset(buffs, 0, 512);
		ParsearHora(buffc, buffs);
		//DameHoraMaquina(buffs);

		printf("Enviando respuesta al cliente... \n");
		int s = send(idsockc, buffs, strlen(buffs), 0);

		if (s == -1)
		{
		    printf("No se pudo enviar la respuesta al cliente");
		    close(idsockc);
		}
	} else if (pid > 0) {
		close(idsockc);
	} else {
		printf("Fork error");
		close(idsockc);
	}
	
    }

    close(idsocks);
}

void DameHoraMaquina(char *buff)
{
    time_t tiempo = time(0);
    tiempo += (5*60);
    struct tm *tlocal = localtime(&tiempo);
    strftime(buff, sizeof(buff), "%H:%M:%S", tlocal);
    //printf("DameHoraMaquina Servidor: %s\n", buff);
}

void ParsearHora(char *buffc, char *buffs) {
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));

    if (strptime(buffc, "%H:%M", &tm) == NULL) {
        fprintf(stderr, "Error al parsear la hora\n");
        return;
    }

    time_t tiempoOriginal = mktime(&tm);
    tiempoOriginal += 5 * 60;

    struct tm *tmModificado = localtime(&tiempoOriginal);

    strftime(buffs, 512, "%H:%M", tmModificado);
}
