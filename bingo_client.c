#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#define MSG_SIZE 250

int
main(int argc, char* argv[]){
	int sd;
	struct sockaddr_in sockname;
	char buffer[MSG_SIZE];
	socklen_t len_sockname;
	fd_set readfds, auxfds;
	int salida;
	int fin = 0;

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd == -1){
		perror("No se puede abrir el socket cliente\n");
		exit(1);
	}

	sockname.sin_family = AF_INET;
	sockname.sin_port = htons(5000);
	sockname.sin_addr.s_addr = inet_addr("127.0.0.1");

	/*solicitar conexion con el servidor*/
	len_sockname = sizeof(sockname);
	if(connect(sd, (struct sockaddr *)&sockname, len_sockname) == -1){
		perror("Error de conexion");
		exit(1);
	}



	FD_ZERO(&auxfds);
	FD_ZERO(&readfds);
	FD_SET(0,&readfds);
	FD_SET(sd,&readfds);


	/*transmitir informacion*/
	do{

		auxfds = readfds;
		salida = select(sd+1, &auxfds, NULL, NULL, NULL);

		if(FD_ISSET(sd, &auxfds)){/*mensaje del servidor*/
			bzero(buffer,sizeof(buffer));
			recv(sd,buffer,sizeof(buffer),0);

			if(strcmp(buffer,"SALIR")==0){
				close(sd);
				return -1;
			}

			else{
				printf("\n%s\n",buffer);
			}

		}else{ /*informacion por teclado*/
			bzero(buffer,sizeof(buffer));
			fgets(buffer,sizeof(buffer),stdin);

			if(strcmp(buffer, "SALIR\n") == 0){
				fin = 1;
			}

			if(strlen(buffer)>0 && strcmp(buffer,"\n")!=0){
				send(sd,buffer,sizeof(buffer),0);
			}

		}

	}while(fin==0);
	close(sd);

}