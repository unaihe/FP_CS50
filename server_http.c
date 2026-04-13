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
	char *secret_message = "The Cheese is in The Toaster";	
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
	
	while(1){
		addr_size = sizeof their_addr;
		int new_fd = accept(sockfd,(struct sockaddr *)&their_addr,&addr_size);
		if (fork() == 0){
			close(sockfd);
			send(new_fd, secret_message, strlen(secret_message),0);
			close(new_fd);
			exit(0);
		}
		else {
			close(new_fd);
		}
	}
		
	return 0;
} 
