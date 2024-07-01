#include <stdio.h>       
#include <stdlib.h>
#include <pthread.h>      
#include <unistd.h>   
#include <string.h>   
#include <fcntl.h>   
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LONGMENSAJE 8*1024

void *atender(void *);

void armarMensajeDirectorio(char *);
void armarMensajeContenidoArchivo(char *);
void armarMensajeCierreConexion(char *);
void borrarArchivo(char *);
void modificarArchivo(char *, int);
void agregarArchivo(char *);

int main(int argc, char * argv[]) {
    if (argc != 3) {
        printf("sintaxis error ingrese ./comando ip puerto\n");
        exit(-1);
    }

    socklen_t addrlen;
    int sockfd;
    struct sockaddr_in addr_in, addrcli_in;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    printf("socket = %d\n", sockfd);

    //completar la estrutura addr_in 
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(atoi(argv[2]));
    addr_in.sin_addr.s_addr = inet_addr(argv[1]);
    memset(addr_in.sin_zero, 0, 8);

    addrlen = sizeof(addr_in);
    int bn = bind(sockfd, (struct sockaddr *)&addr_in, addrlen);
    printf("bind = %d\n", bn);

    int lst = listen(sockfd, 5);
    printf("listen = %d\n", lst);

    while (1) {
        printf("servidor en puerto %s esperando conexion del cliente\n", argv[2]);
        int *sockclifd = malloc(sizeof(int));
        *sockclifd = accept(sockfd, (struct sockaddr *)&addrcli_in, &addrlen);
        if (*sockclifd > 0) {
            pthread_t hilo;
            pthread_create(&hilo, NULL, atender, (void *)sockclifd);
        }
    }
}

void *atender(void *sockclifd) {
    printf("sockclifd = %d\n", *((int *)sockclifd));
    int sockcli = *((int *)sockclifd);
    char mensaje[LONGMENSAJE];
    char opcion;
    do {
        memset(mensaje, '\0', LONGMENSAJE);
        int nb = recv(sockcli, mensaje, LONGMENSAJE, 0);
        mensaje[nb] = '\0';
        opcion = mensaje[0];
        printf("cliente dice -> %s\n", mensaje);
        if (opcion == '1') //el cliente pide directorio 
            armarMensajeDirectorio(mensaje);
        if (opcion == '2') //el cliente pide un archivo
            armarMensajeContenidoArchivo(mensaje);
        if (opcion == '3') // El cliente pide borrar un archivo
            borrarArchivo(mensaje);
        if (opcion == '4') // El cliente pide modificar un archivo
            modificarArchivo(mensaje, sockcli);
        if (opcion == '5') // El cliente pide agregar un archivo
            agregarArchivo(mensaje);
        if (opcion == '0') // cliente pide cerrar conexion
            armarMensajeCierreConexion(mensaje);
        send(sockcli, mensaje, strlen(mensaje), 0);
    } while (opcion != '0');
    close(sockcli);
    free(sockclifd);
    pthread_exit(NULL);
}

void armarMensajeDirectorio(char *mensaje) {
    memset(mensaje, '\0', LONGMENSAJE);
    DIR *dir = opendir(".");
    if (dir != NULL) {
        struct dirent *midirent;
        struct stat mistat;
        while ((midirent = readdir(dir)) != NULL) {
            printf("___Registro del directorio archivo_______\n");
            printf("inodo %d\n", midirent->d_ino);   /* Inode number */
            printf("largo registro %d\n", midirent->d_reclen); /*Length of this record */
            printf("type %d\n", midirent->d_type);      /* Type of file; not supported */
            printf("nombre %s\n", midirent->d_name);    /* Null-terminated filename */
            printf("__________________________________\n");
            if (stat(midirent->d_name, &mistat) != -1) {
                printf("     __informacion del archivo___\n");
                printf("     inode %d\n", mistat.st_ino); /* Inode number */
                printf("     mode %d\n", mistat.st_mode);/* File type and mode */
                printf("     links %d\n", mistat.st_nlink);/* Number of hard links */
                printf("     uid %d\n", mistat.st_uid);  /* User ID of owner */
                printf("     gid %d\n", mistat.st_gid);  /* Group ID of owner */
                printf("     tamaño %ld\n", mistat.st_size); /* Total size, in bytes */
                printf("     __________________________________\n");
            }
            char sizeArchivo[16];
            memset(sizeArchivo, '\0', 16);
            strcat(mensaje, midirent->d_name);
            strcat(mensaje, " ");
            sprintf(sizeArchivo, "%ld", mistat.st_size);
            strcat(mensaje, sizeArchivo);
            strcat(mensaje, "\n");
        }
        strcat(mensaje, "\0");
        closedir(dir);
    }
}

void armarMensajeContenidoArchivo(char *mensaje) {
    int i, j;
    char *nombreArchivo, *dir;
    nombreArchivo = (char *)malloc(64);
    dir = nombreArchivo;
    j = 0;
    for (i = 1; mensaje[i] != '\0' && mensaje[i] != '\n'; i++) {
        *dir = mensaje[i];
        dir++;
    }
    *dir = '\0';
    memset(mensaje, '\0', LONGMENSAJE);
    struct stat mistat;
    if (stat(nombreArchivo, &mistat) != -1) {
        int fd = open(nombreArchivo, O_RDONLY);
        int nb = read(fd, mensaje, mistat.st_size);
        mensaje[nb] = '\0';
        close(fd);
    }
    free(nombreArchivo);
}

void armarMensajeCierreConexion(char *mensaje) {
    memset(mensaje, '\0', LONGMENSAJE);
    strcat(mensaje, "chau cliente\0");
}

void agregarArchivo(char *mensaje) {
    char *nombreArchivo = mensaje + 1; // El nombre del archivo comienza desde el segundo caracter
    FILE *archivo = fopen(nombreArchivo, "w");
    if (archivo != NULL) {
        fclose(archivo);
        sprintf(mensaje, "Archivo %s creado exitosamente", nombreArchivo);
    } else {
        perror("Error al crear archivo"); // Imprimir error detallado
        sprintf(mensaje, "No se pudo crear el archivo %s", nombreArchivo);
    }
}

void borrarArchivo(char *mensaje) {
    char *nombreArchivo = mensaje + 1; // El nombre del archivo comienza desde el segundo caracter
    int resultado = remove(nombreArchivo);
    if (resultado == 0) {
        printf("Archivo borrado exitosamente: %s\n", nombreArchivo);
        strcpy(mensaje, "Archivo borrado exitosamente");
    } else {
        printf("No se pudo borrar el archivo: %s\n", nombreArchivo);
        strcpy(mensaje, "No se pudo borrar el archivo");
    }
}

void modificarArchivo(char *mensaje, int sockcli) {
    printf("Antes de while");
    char *nombreArchivo = mensaje + 1; // El nombre del archivo comienza desde el segundo caracter
    char contenidoNuevo[LONGMENSAJE];
    int nb, fd, nbytes;

    // Abrir el archivo en modo de escritura
    fd = open(nombreArchivo, O_WRONLY | O_TRUNC);
    if (fd == -1) {
        // Si no se pudo abrir el archivo, envía un mensaje de error al cliente
        strcpy(mensaje, "No se pudo abrir el archivo para modificarlo");
        send(sockcli, mensaje, strlen(mensaje), 0);
        return;
    }
    
    printf("Antes de while");
    strcpy(mensaje, "Archivo abierto con exito");
    send(sockcli, mensaje, strlen(mensaje), 0);

    // Recibir el nuevo contenido del archivo del cliente
    memset(contenidoNuevo, '\0', LONGMENSAJE);
    printf("Antes de while");
    while ((nb = recv(sockcli, contenidoNuevo, LONGMENSAJE, 0)) > 0) {
        printf("Contenido recibido");
        // Escribir el contenido recibido en el archivo
        nbytes = write(fd, contenidoNuevo, nb);
        if (nbytes == -1) {
            // Si hubo un error al escribir en el archivo, envía un mensaje de error al cliente
            strcpy(mensaje, "Error al escribir en el archivo");
            close(fd);
            send(sockcli, mensaje, strlen(mensaje), 0);
            return;
        }
    }

    // Cerrar el archivo
    close(fd);

    // Enviar mensaje de confirmación al cliente
    strcpy(mensaje, "Archivo modificado exitosamente");
    send(sockcli, mensaje, strlen(mensaje), 0);
}

