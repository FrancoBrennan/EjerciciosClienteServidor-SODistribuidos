#include <stdio.h>       
#include <stdlib.h>
#include <unistd.h>   
#include <string.h>   
#include <fcntl.h>   

#include <sys/types.h>       
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LONGMENSAJE 8*1024

void enviameDirectorio(int);
void enviameUnArchivo(int);
void cierraConexion(int);
void borrarArchivo(int);
void modificarArchivo(int);
void agregarArchivo(int);

int menu(void);

int main(int argc, char * argv[])
{
    if (argc != 3 )
     {
       printf("sintaxis error ingrese ./comando ip puerto\n");
       exit(-1) ;
     }
     int sockclifd ;
     struct sockaddr_in addrcli_in ; 
     sockclifd = socket(AF_INET, SOCK_STREAM , 0);
     printf("socket %d \n",sockclifd);     

     //completar la estructura addr_in 
     addrcli_in.sin_family = AF_INET ;
     addrcli_in.sin_port   = htons(atoi(argv[2])) ;
     addrcli_in.sin_addr.s_addr  = inet_addr(argv[1]) ;
     memset(addrcli_in.sin_zero,0,8) ;
      
     int error = connect(sockclifd, (struct sockaddr *)&addrcli_in, (socklen_t)sizeof(addrcli_in));
     if ( error == -1 )
        printf("ERROR CONNECT\n");
     else    
         {
              printf("Cliente conectado con el servidor\n");
              int opcion ;
              while((opcion = menu()) != 0)
              {
                 switch(opcion)
                 {
                    case 0:
                         {
                           cierraConexion(sockclifd);
                           break;
                         }
                    case 1:
                         {
                           enviameDirectorio(sockclifd);     
                           break;
                         }
                    case 2: 
                         {
                           enviameUnArchivo(sockclifd);     
                           break;
                         }
                    case 3: 
                    	{
                    	   borrarArchivo(sockclifd); 
                    	   break;
                    	}
                    case 4:
                        {	
			   modificarArchivo(sockclifd);
			   break;                    
                        }
                    case 5:
                        {
                           agregarArchivo(sockclifd);
                           break;
                        }
                 }  
              }
              cierraConexion(sockclifd);
              close(sockclifd);
         }
}


void enviameDirectorio(int sockclifd)
{                       
              char mensaje[LONGMENSAJE];
              memset(mensaje,'\0',LONGMENSAJE);
              strcat(mensaje,"1");
              send(sockclifd,mensaje,strlen(mensaje),0);
              memset(mensaje,'\0',LONGMENSAJE);
              int nb = recv(sockclifd,mensaje,LONGMENSAJE,0);
              mensaje[nb] = '\0';
              printf("Archivos del directorio de trabajo->\n%s\n",mensaje);
}

void enviameUnArchivo(int sockclifd)
{
  char nombreArchivo[64];
  memset(nombreArchivo,'\0',64);
  char mensaje[LONGMENSAJE];
  memset(mensaje,'\0',LONGMENSAJE);
  strcat(mensaje,"2");
  printf("Ingrese el nombre del archivo--> ");
  scanf("%s",nombreArchivo);  
  strcat(mensaje,nombreArchivo);
  strcat(mensaje,"\0");
  send(sockclifd,mensaje,strlen(mensaje),0);
  memset(mensaje,'\0',LONGMENSAJE);
  int nb = recv(sockclifd,mensaje,LONGMENSAJE,0);
  mensaje[nb] = '\0';
  printf("Contenido del Archivo->\n%s\n",mensaje);
}

void borrarArchivo(int sockclifd)
{
    char nombreArchivo[64];
    memset(nombreArchivo, '\0', 64);
    char mensaje[LONGMENSAJE];
    memset(mensaje, '\0', LONGMENSAJE);
    strcat(mensaje, "3"); // Comando para borrar un archivo
    printf("Ingrese el nombre del archivo a borrar: ");
    scanf("%s", nombreArchivo);
    strcat(mensaje, nombreArchivo);
    strcat(mensaje, "\0");
    send(sockclifd, mensaje, strlen(mensaje), 0);
    memset(mensaje, '\0', LONGMENSAJE);
    int nb = recv(sockclifd, mensaje, LONGMENSAJE, 0);
    mensaje[nb] = '\0';
    printf("%s\n", mensaje);
}

void cierraConexion(int sockclifd)
{
  char mensaje[LONGMENSAJE];
  memset(mensaje,'\0',LONGMENSAJE);
  strcpy(mensaje,"0\0");
  send(sockclifd,mensaje,strlen(mensaje),0);
  memset(mensaje,'\0',LONGMENSAJE);
  int nb = recv(sockclifd,mensaje,LONGMENSAJE,0);
  mensaje[nb] = '\0';
  printf("Cierre de conexion->\n%s\n",mensaje);
}

void agregarArchivo(int sockclifd)
{
    char nombreArchivo[64];
    memset(nombreArchivo, '\0', 64);
    char mensaje[LONGMENSAJE];
    memset(mensaje, '\0', LONGMENSAJE);
    strcat(mensaje, "5"); // Comando para agregar un archivo
    printf("Ingrese el nombre del archivo a agregar: ");
    scanf("%s", nombreArchivo);
    strcat(mensaje, nombreArchivo);
    strcat(mensaje, "\0");
    send(sockclifd, mensaje, strlen(mensaje), 0);
    memset(mensaje, '\0', LONGMENSAJE);
    int nb = recv(sockclifd, mensaje, LONGMENSAJE, 0);
    mensaje[nb] = '\0';
    printf("%s\n", mensaje);
}

void modificarArchivo(int sockclifd)
{
    char nombreArchivo[64];
    memset(nombreArchivo, '\0', 64);
    char mensaje[LONGMENSAJE];
    memset(mensaje, '\0', LONGMENSAJE);
    strcat(mensaje, "4"); // Comando para modificar un archivo
    printf("Ingrese el nombre del archivo a modificar: ");
    scanf("%s", nombreArchivo);
    strcat(mensaje, nombreArchivo);
    strcat(mensaje, "\0");
    
    // Envía el nombre del archivo al servidor
    send(sockclifd, mensaje, strlen(mensaje), 0);
    
    // Recibe el contenido actual del archivo del servidor
    memset(mensaje, '\0', LONGMENSAJE);
    int nb = recv(sockclifd, mensaje, LONGMENSAJE, 0);
    mensaje[nb] = '\0';
    printf("Contenido actual del archivo:\n%s\n", mensaje);
    
    // Solicita el nuevo contenido al usuario
    printf("Ingrese el nuevo contenido del archivo (. para finalizar):\n");
    memset(mensaje, '\0', LONGMENSAJE);
    char input[LONGMENSAJE];
    /*while (1) {
        fgets(input, LONGMENSAJE, stdin);
        // Elimina el carácter de nueva línea del final de la cadena
        input[strcspn(input, "\n")] = '\0';
        if (strstr(input, ".") != NULL) {
        // Si el "." está presente en el input, termina la entrada
        break;
        }
    }*/
    
    while (fgets(input, LONGMENSAJE, stdin) != NULL) {
    	// Elimina el carácter de nueva línea del final de la cadena
    	input[strcspn(input, "\n")] = '\0';
    
	// Verifica si la cadena de entrada contiene el "."
    	if (strstr(input, ".") != NULL) {
        	// Si el "." está presente en la entrada, termina la entrada
        	strcat(mensaje, input);  // Agrega la línea con el "."
        	break;
    	} else {
        	// Si el "." no está presente, agrega la línea al mensaje
		strcat(mensaje, input);
    	}
    }
    
    printf("ya termino el while");
    
    // Envía el nuevo contenido al servidor
    send(sockclifd, mensaje, strlen(mensaje), 0);
    
    // Recibe la confirmación del servidor
    printf("Recibiendo la confirmacion del servidor....");
    memset(mensaje, '\0', LONGMENSAJE);
    nb = recv(sockclifd, mensaje, LONGMENSAJE, 0);
    mensaje[nb] = '\0';
    printf("%s\n", mensaje);
    
    // Salir de la función después de enviar el contenido al servidor
    return;
}







int menu(void)
{
  printf("Menu de opciones del Cliente\n");
  printf("1 Enviame el Directorio\n");
  printf("2 Pedir un archivo\n");
  printf("3 Borrar un archivo\n");
  printf("4 Modificar un archivo\n");
  printf("5 Agregar un archivo\n");
  printf("0 Cerrar conexion\n");
  printf("_________________\n");
  printf("Ingrese opcion: \n");
  int opcion ;
  scanf("%d",&opcion);
  return opcion;
}
