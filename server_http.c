//Librerias necesarias para las funciones de socket
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>       // Para fprintf, perror, stderr
#include <stdlib.h>      // Para exit()
#include <unistd.h>      // Para fork(), close()
#include <errno.h>       // Para errno
#include <string.h>      // Para memset(), strlen()
#include <sys/types.h>   // Para tipos de datos del sistema
#include <sys/socket.h>  // Para socket(), bind(), accept(), etc.
#include <netdb.h>       // Para getaddrinfo, struct addrinfo
#include <sys/wait.h>    // Para waitpid() y WNOHANG
#include <signal.h>      // Para sigaction, SIGCHLD, SA_RESTART

//Gestion de hijos (limpieza)
void sigchld_handler(int s)
{
 (void)s; 

 int saved_errno = errno;

 while(waitpid(-1, NULL, WNOHANG) > 0);

 errno = saved_errno;
 }

int main(){
	int sockfd;              
    	struct addrinfo *p;
	//Estructura de acciones a seguir cuando muere un hijo
	struct sigaction sa;
	//PAra guardar la direccion entrante
	struct sockaddr_storage their_addr; 
	socklen_t addr_size;
	int status;
	//Creamos la estructura que usaremos para guardar todos los datos que tiene que cumplir nuestro servidor
	struct addrinfo hints; 
	//Creamos otra estructura de salida
	struct addrinfo *servinfo;
	//Ponemos los valores que buscamos en la estructura
	memset(&hints, 0, sizeof hints);
	hints.ai_socktype=SOCK_STREAM; 
	hints.ai_flags=AI_PASSIVE; 

	if (status=getaddrinfo(NULL,"3490",&hints,&servinfo)!=0){
		fprintf(stderr, "gai error: %s\n", gai_strerror(status));
		exit(1);
	}

	for(p=servinfo;p!=NULL;p=p->ai_next){
		//Probamos con la opción actual
		sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		//Evitar errores al reiniciar
    		int yes = 1;
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	    	// Si sockfd es -1, es que ha fallado este intento, pasamos al siguiente
	    	if (sockfd == -1) {
			continue; 
	   	}	
		
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd); // Si el socket se creó pero el puerto está ocupado, cerramos y seguimos buscando
			continue;
	    	}
	    	break;
	}
	freeaddrinfo(servinfo);
	listen(sockfd, 10);
	
	sa.sa_handler=sigchld_handler;
	sigemptyset(&sa.sa_mask); 
	sa.sa_flags= SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
 		exit(1);
	}
	
	while(1) {
		addr_size = sizeof their_addr;
		int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
		
		if (fork() == 0) { // Proceso HIJO
		    close(sockfd);
		    
		    char request[2048];
		    char method[10], path[256], protocol[10];

		    // Recibimos los datos del navegador
		    int bytes_recibidos = recv(new_fd, request, sizeof(request)-1, 0);
		    
		    if (bytes_recibidos > 0) {
		        request[bytes_recibidos] = '\0';
		        printf("Petición:\n%s\n", request);

		        //Analizamos qué archivo quiere
		        sscanf(request, "%s %s %s", method, path, protocol);
		        
		        char *file_name = path + 1; // Saltamos la '/'
		        if (strlen(file_name) == 0) {
		            file_name = "index.html"; // Por defecto
		        }

		        //Intentamos abrir el archivo solicitado
		        FILE *file = fopen(file_name, "r");

		        if (file == NULL) {
		            char *error404 = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 22\r\n\r\n<h1>404 Not Found</h1>";
		            send(new_fd, error404, strlen(error404), 0);
		        } else {
		            fseek(file, 0, SEEK_END);
		            long fsize = ftell(file);
		            rewind(file);

		            char *contenido = malloc(fsize + 1);
		            fread(contenido, 1, fsize, file);
		            contenido[fsize] = '\0';

		            char response[512];
		            sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\nConnection: close\r\n\r\n", fsize);
		            
		            send(new_fd, response, strlen(response), 0);
		            send(new_fd, contenido, fsize, 0);

		            free(contenido);
		            fclose(file);
		        }
		    }
		    close(new_fd);
		    exit(0);
		}
		close(new_fd); // Proceso PADRE
	    }
	
		
	return 0;
} 
