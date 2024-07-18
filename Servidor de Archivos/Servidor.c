#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>

#define MAX_BUFFER 1024
#define IP "127.0.0.1"
#define PUERTO 8888

#define MAX_THREADS 2

void *atender_cliente(void *);

sem_t sem;

int main(int argc, char *argv[])
{
    socklen_t addrlen;
    int idsocks;
    struct sockaddr_in addr_in, addrcli_in;

    idsocks = socket(AF_INET, SOCK_STREAM, 0);
    if (idsocks < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    int puerto = atoi(argv[1]);
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(PUERTO);
    addr_in.sin_addr.s_addr = inet_addr(IP);
    memset(addr_in.sin_zero, 0, sizeof(addr_in.sin_zero));

    addrlen = sizeof(addr_in);
    if (bind(idsocks, (struct sockaddr *)&addr_in, addrlen) < 0) {
        perror("Error en bind");
        close(idsocks);
        exit(EXIT_FAILURE);
    }

    if (listen(idsocks, MAX_THREADS) < 0) {
        perror("Error en listen");
        close(idsocks);
        exit(EXIT_FAILURE);
    }

    printf("Esperando conexiones en el puerto %d...\n", puerto);

    sem_init(&sem, 0, MAX_THREADS);

    while (1)
    {
        printf("Esperando conexión de cliente...\n");

        int *idsockc = malloc(sizeof(int));
        if (!idsockc) {
            perror("Error al asignar memoria");
            continue;
        }

        *idsockc = accept(idsocks, (struct sockaddr *)&addrcli_in, &addrlen);

        if (*idsockc == -1)
        {
            perror("Error en accept");
            free(idsockc);
            continue;
        }

        // Intentar adquirir el semáforo para asegurar que no se superen los MAX_THREADS
        if (sem_trywait(&sem) != 0)
        {
            printf("Servidor lleno. Informando al cliente...\n");
            char msg[] = "Servidor lleno, por favor intente más tarde.";
            send(*idsockc, msg, strlen(msg), 0);
            close(*idsockc);
            free(idsockc);
            continue;
        }
        else{
            char msg[] = "Servidor LIBRE.";
            send(*idsockc, msg, strlen(msg), 0);
        }

        printf("Conexión aceptada desde el cliente %d\n", *idsockc);

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, atender_cliente, idsockc) != 0)
        {
            perror("Error al crear el hilo");
            close(*idsockc);
            free(idsockc);
            sem_post(&sem);
        }
        else
        {
            pthread_detach(thread_id);
        }
    }

    sem_destroy(&sem);
    close(idsocks);
    return 0;
}

void *atender_cliente(void *idsockcli)
{
    int idsockc = *(int *)idsockcli;
    free(idsockcli);
    char buffer[MAX_BUFFER];
    memset(buffer, 0, sizeof(buffer));

    while (1)
    {
        int nb = recv(idsockc, buffer, sizeof(buffer) - 1, 0);
        if (nb > 0)
        {
            buffer[nb] = '\0';
            printf("Solicitud del cliente: %s\n", buffer);

            if (buffer[0] == '1')
            {
                DIR *dir = opendir(".");
                if (dir != NULL)
                {
                    struct dirent *midirent;
                    struct stat mistat;
                    while ((midirent = readdir(dir)) != NULL)
                    {
                        if (stat(midirent->d_name, &mistat) == 0)
                        {
                            snprintf(buffer, sizeof(buffer), "%s %ld\n", midirent->d_name, mistat.st_size);
                            send(idsockc, buffer, strlen(buffer), 0);
                        }
                    }
                    closedir(dir);
                }
                else
                {
                    perror("Error al abrir el directorio");
                }
                strcpy(buffer, "\nEND\n");
                send(idsockc, buffer, strlen(buffer), 0);
            }
            else if (buffer[0] == '2')
            {
                char *filename = buffer + 1;
                filename[strcspn(filename, "\n")] = '\0';
                int fd = open(filename, O_RDONLY);
                if (fd < 0)
                {
                    perror("Error al abrir el archivo");
                    strcpy(buffer, "Error al abrir el archivo");
                    send(idsockc, buffer, strlen(buffer), 0);
                }
                else
                {
                    int bytes_read;
                    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0)
                    {
                        buffer[bytes_read] = '\0';
                        send(idsockc, buffer, bytes_read, 0);
                    }
                    close(fd);
                }
                strcpy(buffer, "\nEND\n");
                send(idsockc, buffer, strlen(buffer), 0);
            }
            else if (buffer[0] == '3')
            {
                char *filename = buffer + 1;
                char *content = strstr(buffer, "\n") + 1;
                filename[strcspn(filename, "\n")] = '\0';

                char old_content[MAX_BUFFER];
                int fd = open(filename, O_RDWR);
                if (fd < 0)
                {
                    perror("Error al abrir el archivo");
                    strcpy(buffer, "Error al abrir el archivo");
                    send(idsockc, buffer, strlen(buffer), 0);
                }
                else
                {
                    int bytes_read = read(fd, old_content, sizeof(old_content) - 1);
                    old_content[bytes_read] = '\0';

                    char *line = strtok(content, "\n");
                    while (line != NULL)
                    {
                        if (strstr(line, "agregar ") == line)
                        {
                            line += strlen("agregar ");
                            strcat(old_content, line);
                            strcat(old_content, "\n");
                        }
                        else if (strstr(line, "eliminar ") == line)
                        {
                            line += strlen("eliminar ");
                            char *pos = strstr(old_content, line);
                            if (pos != NULL)
                            {
                                char *end_of_line = strstr(pos, "\n");
                                if (end_of_line != NULL)
                                {
                                    memmove(pos, end_of_line + 1, strlen(end_of_line + 1) + 1);
                                }
                                else
                                {
                                    *pos = '\0';
                                }
                            }
                        }
                        else
                        {
                            char camiseta[64];
                            int dorsal;
                            sscanf(line, "%s %d", camiseta, &dorsal);
                            char *pos = strstr(old_content, camiseta);
                            if (pos != NULL)
                            {
                                char *end_of_line = strstr(pos, "\n");
                                if (end_of_line != NULL)
                                {
                                    memmove(pos, end_of_line + 1, strlen(end_of_line + 1) + 1);
                                }
                                else
                                {
                                    *pos = '\0';
                                }
                                strcat(old_content, camiseta);
                                strcat(old_content, " ");
                                char dorsal_str[10];
                                sprintf(dorsal_str, "%d", dorsal);
                                strcat(old_content, dorsal_str);
                                strcat(old_content, "\n");
                            }
                        }
                        line = strtok(NULL, "\n");
                    }

                    lseek(fd, 0, SEEK_SET);
                    ftruncate(fd, 0);
                    if (write(fd, old_content, strlen(old_content)) < 0)
                    {
                        perror("Error al escribir en el archivo");
                        strcpy(buffer, "Error al escribir en el archivo");
                        send(idsockc, buffer, strlen(buffer), 0);
                    }
                    else
                    {
                        strcpy(buffer, "Archivo modificado con éxito");
                        send(idsockc, buffer, strlen(buffer), 0);
                    }
                    close(fd);
                }
                strcpy(buffer, "\nEND\n");
                send(idsockc, buffer, strlen(buffer), 0);
            }
            else if (buffer[0] == '4')
            {
                char *filename = buffer + 1;
                filename[strcspn(filename, "\n")] = '\0';

                if (remove(filename) == 0)
                {
                    strcpy(buffer, "Archivo eliminado con éxito");
                }
                else
                {
                    perror("Error al eliminar el archivo");
                    strcpy(buffer, "Error al eliminar el archivo");
                }
                send(idsockc, buffer, strlen(buffer), 0);
                strcpy(buffer, "\nEND\n");
                send(idsockc, buffer, strlen(buffer), 0);
            }
            else if (buffer[0] == '5')
            {
                char *filename = buffer + 1;
                char *content = strstr(buffer, "\n") + 1;
                filename[strcspn(filename, "\n")] = '\0';

                int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (fd < 0)
                {
                    perror("Error al crear el archivo");
                    strcpy(buffer, "Error al crear el archivo");
                    send(idsockc, buffer, strlen(buffer), 0);
                }
                else
                {
                    if (write(fd, content, strlen(content)) < 0)
                    {
                        perror("Error al escribir en el archivo");
                        strcpy(buffer, "Error al escribir en el archivo");
                        send(idsockc, buffer, strlen(buffer), 0);
                    }
                    else
                    {
                        strcpy(buffer, "Archivo creado con éxito");
                        send(idsockc, buffer, strlen(buffer), 0);
                    }
                    close(fd);
                }
                strcpy(buffer, "\nEND\n");
                send(idsockc, buffer, strlen(buffer), 0);
            }
            else if (buffer[0] == '9')
            {
                strcpy(buffer, "Conexión cerrada");
                send(idsockc, buffer, strlen(buffer), 0);
                printf("Cliente desconectado\n");
                break;
            }
        }
        else if (nb == 0)
        {
            printf("Cliente desconectado\n");
            break;
        }
        else
        {
            perror("Error al recibir el mensaje");
            break;
        }
        memset(buffer, 0, sizeof(buffer));
    }

    close(idsockc);
    sem_post(&sem); // Liberar el semáforo al finalizar la conexión
    return NULL;
}

