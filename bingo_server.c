#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bingo_server.h"
#include "partida.h"
#include "jugador.h"
#include "cartones.h"


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <signal.h>

int
main(int argc, char* argv[]){
	int i,j;

	static Partida* partida[10];
	static Jugador* jugador[40];
	static Command* cmd_head = NULL;

	cmd_ini(&cmd_head);

	/*registro comandos que voy a usar*/
	cmd_reg(&cmd_head, "WHO", &cb_who);
	cmd_reg(&cmd_head, "USUARIO", &cb_usuario);
	cmd_reg(&cmd_head, "PASSWORD", &cb_password);


	/*variables auxiliares para registrar jugadores*/
	char reg_username[40];
	char reg_pass[128];

	int sd, new_sd;
	struct sockaddr_in sockname, from;

	char buffer[MSG_SIZE];
	char * pch; /*para strtok*/

	socklen_t from_len;
	struct hostent * host;
	fd_set readfds, auxfds;
	int on, ret;
	int salida;
	int recibidos;
	int nconexiones=0; /*clientes conectados*/
	char identificador[MSG_SIZE];
	int todos_listos;
	int keep_going=1;
	int bola;

	char micarton[170];
	char num[2]; /*para imprimir numero carton*/


	/*inicializar punteros a NULL*/
	for(i=0;i<40;i++){
		if(i<10)
			partida[i]=NULL;
		jugador[i]=NULL;
	}

	/*abrir socket*/
	sd = socket(AF_INET,SOCK_STREAM, 0);
	if(sd==-1){
		perror("Error al abrir socket cliente.\n");
		exit(1);
	}

	/**
	 * activar SO_REUSEADDR
	 * para cuando se cierra el servidor, no tengamos que esperar
	 * el tiempo
	 */
	on=1;
	ret = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	sockname.sin_family = AF_INET;
	sockname.sin_port = htons(5000);
	sockname.sin_addr.s_addr = INADDR_ANY;

	if(bind(sd,(struct sockaddr *) &sockname, sizeof(sockname)) == -1){
		perror("Error en la operacion bind");
		exit(1);
	}

	/**
	 * De las peticiones a aceptar solo necesitamos
	 * tamano de su estructura, el resto de info
	 * (familia, puerto,  ip) la da el metodo que
	 * recibe las peticiones.
	 */
	from_len = sizeof(from);

	if(listen(sd,1) == -1){
		perror("Error en listen");
		exit(1);
	}

	/*inicializar conjuntos fdset*/
	FD_ZERO(&readfds);
	FD_ZERO(&auxfds);
	FD_SET(sd, &readfds);
	FD_SET(0,&readfds);

	signal(SIGINT, manejador); /*para capturar CTRL+C*/
	signal(SIGALRM, sacar_bolas);


	/*servidor acepta una peticion*/
	while(1){

		/*interrupcion periodica para sacar bolas*/
		alarm(6);
		if(sacar==1){
			signal(SIGALRM, SIG_IGN);

			/*sacar bolas*/
			for(i=0;i<10;i++){
				if(partida[i]!=NULL){

					if(partida[i]->bombo == NULL){
						/**
						 * enviar mensaje de partida terminada
						 * sacar jugadores de la partida
						 * liberar memoria
						 */

						 partida_clean(&partida[i]);
					}

					if(partida[i]->iniciada == 1){
						bola = partida_sacar(&partida[i]);
						if(bola>0){
							for(j=0;j<4;j++){
								if(partida[i]->jugadores[j]!=NULL){
									bzero(buffer,sizeof(buffer));
									sprintf(buffer,"\nEl %d!\n",bola);
									send(partida[i]->jugadores[j]->id,buffer,strlen(buffer),0);
								}
							}
						}
					}

				}
			}/*endfor*/

			sacar = 0;
			signal(SIGALRM, sacar_bolas);
		}

		/**
		 * Esperar mensajes de clientes: nuevas conexiones o
		 * mensajes de clientes ya conectados
		 */
		auxfds = readfds;
		salida = select(FD_SETSIZE, &auxfds, NULL, NULL, NULL);
		if(salida > 0){
			for(i=0;i<FD_SETSIZE;i++){

				/*buscar socket por donde se establecio comunicacion*/
				if(FD_ISSET(i, &auxfds)){
					if(i==sd){ /*nueva conexion*/

						if((new_sd = accept(sd, (struct sockaddr *)&from, &from_len)) == -1){
							perror("Error aceptando peticiones.");
						}
						else{

							ret = ingresar_jugador(jugador);
							if(ret==-1){

								bzero(buffer,sizeof(buffer));
								strcpy(buffer,"-ERR. Demasiados usuarios");
								send(new_sd, buffer, strlen(buffer),0);
								close(new_sd);

							}else{

								/*le asigno al nuevo jugador su numero de socket*/
								jugador[ret]->id = new_sd;

								FD_SET(new_sd, &readfds);
								strcpy(buffer,"+Ok. Usuario conectado");
								send(new_sd, buffer, strlen(buffer),0);

							}

						}

					}
					else if(i==0){ /*se ha introducido info. teclado*/
						bzero(buffer, sizeof(buffer));
						fgets(buffer, sizeof(buffer), stdin);

						if(strcmp(buffer,"SALIR\n") == 0 || salir == 1){

							i=0;
							while(i<10){
								if(partida[i]!=NULL){
									free(partida[i]);
									partida[i]=NULL;
								}
								i++;
							}


							// enviar a todos los clientes salir
							i=0;
							while(i<40){
								if(jugador[i]!=NULL){
									send(jugador[i]->id,"SALIR",5,0);
									free(jugador[i]);
									jugador[i]=NULL;
								}
								i++;
							}


							close(sd);
							exit(-1);
						}

						if(strcmp(buffer,"CLIENTS\n") == 0){
							printf("id\tslot\tusername\n");
							for(i=0;i<MAX_CLIENTS;i++){

								if(jugador[i]!=NULL){
									printf("%d\t%d\t%s\n",i,jugador[i]->id,jugador[i]->username);
								}
								else{
									//printf("%d\tX\tX\n",i);
								}

							}
						}
					}

					else{
						bzero(buffer, sizeof(buffer));
						recibidos = recv(i,buffer,sizeof(buffer),0);


						if(recibidos>0){


							ret=buscar_jugador(jugador,i);

							/*ejecuta la funcion que le corresponda*/
							if(jugador[ret]->id_partida == -1){
								printf("No tiene partida\n");
								cmd_exe(cmd_head, buffer, &jugador[ret], NULL);
							}
							else
								cmd_exe(cmd_head, buffer, &jugador[ret], &partida[jugador[ret]->id_partida]);

						}
					}
			}
		}

	}

}



return EXIT_SUCCESS;
}




void manejador(int signum){
	salir = 1;
}

void sacar_bolas(int signum){
	signal(SIGALRM, SIG_IGN);

	sacar = 1;

	signal(SIGALRM, sacar_bolas);
}

/**
 * Buscar partida libre a un jugador. si la encuentra,
 * lo mete y devuelve el slot de la partida. Si no devuelve -1.
 */
int buscar_partida(Partida* partida[10], Jugador* jugador){
	int i;

	/*primero buscamos salas con gente QUE NO ESTEN INICIADAS*/
	for(i=0;i<10;i++){
		if(partida[i]!=NULL){
			if( partida[i]->njugadores < 4  && partida[i]->iniciada == 0){
				partida_ingresar(&partida[i], &jugador);
				return i;
			}
		}
	}

	/*despues buscamos salas vacias*/
	for(i=0;i<10;i++){
		if(partida[i]==NULL){
			partida_nueva(i, &partida[i]);
			partida_ingresar(&partida[i], &jugador);
			return i;
		}
	}

	/*si llegamos aqui esta todo completo*/
	return -1;
}

/**
 * Nuevo jugador en el sistema.
 */
int ingresar_jugador(Jugador* jugador[40]){
	int i;

	/*TODO pedir usuario y pass*/

	for(i=0;i<40;i++){
		if(jugador[i] == NULL ){
			jugador_nuevo(&jugador[i]);
			return i;
		}
	}

	return -1;
}

/** Buscar jugador en el sistema por su descriptor
 *  Devuelve su posicion en el array o -1.
 */
int buscar_jugador(Jugador* j[],int id){
	int i;
	for(i=0;i<MAX_CLIENTS;i++){
		if(j[i]->id == id){
			return i;
		}
	}
	return -1;
}


/**
 * cmd_ini inicializa la lista enlazada de comandos
 */
void cmd_ini(Command** cmd_head){

	(*cmd_head) = NULL;
}

/**
 * cmd_reg introduce un nuevo comando en la lista
 */
void cmd_reg(Command** cmd_head, char *buffer, void (*cb)(char *buffer, Jugador** j, Partida** p)){
	Command *cursor;

	if(*cmd_head == NULL){

		/*registrar primer comando*/
		*cmd_head = (Command*) malloc(sizeof(Command));
		(*cmd_head)->next = NULL;
		strcpy( (*cmd_head)->buffer, buffer );
		(*cmd_head)->cb = (*cb);

	}

	else{
		cursor = *cmd_head;

		while(cursor->next != NULL){
			/*salir cuando estemos en el ultimo*/
			cursor = cursor->next;
		}

		/*registrar comando en la ultima posicion de la lista*/

		cursor->next = (Command*) malloc(sizeof(Command));

		cursor->next->next = NULL;
		strcpy(cursor->next->buffer, buffer);
		cursor->next->cb = (*cb);
	}

}

/**
 * cmd_exe busca comando que coincide con cadena str, y ejecuta su función
 * cmd_head -> cabeza lista de comandos disponibles
 * str -> cadena caracteres que identifica el comando a ejecutar
 * sd -> descriptor de socket que quiere ejecutar el comando
 */
int cmd_exe(Command* cmd_head, char *buffer, Jugador** j, Partida** p){
	char *id; /*cadena que identifica el comando a ejecutar*/
	char *args; /*resto del buffer*/

	printf("aqui\n");

	id = strtok(buffer," \n"); /*extraer comando*/
	args = strtok(NULL, "\n");




	while(cmd_head!=NULL && strcmp(cmd_head->buffer,id)!=0){
		/* pone cmd_head al nodo que coincide con comando dentro de buffer,
	   * o al final de la lista. */
		printf("%s|%s\n",id,cmd_head->buffer);
		cmd_head = cmd_head->next;
	}

	if(cmd_head != NULL){
		/*pongo en buffer los argumentos (puede no tener argumentos)*/
		if(args!=NULL){
			strcpy(buffer, args);
			/*ejecutar funcion indicada en el nodo*/
			cmd_head->cb(buffer,j,p);
		}
		else{
			cmd_head->cb(NULL,j,p);
		}

		return 1;
	}
	else{
		return 0;
	}
}

/**
 * Limpia memoria de la lista
 */
void cmd_clean(Command** cmd_head){
	Command *aux = *cmd_head;

	if(*cmd_head==NULL){
		return;
	}

	while(*cmd_head != NULL){
		aux = (*cmd_head)->next;
		free(*cmd_head);
		*cmd_head = aux;
	}
}

/**
 * Function: cb_who
 * --------------------
 *
 *	Responde al jugador que lo solicita su nombre de usuario y el numero
 *  de socket.
 *
 *  char *args    NULL
 *  Jugador** j   jugador al que responder
 *  Partida** p   partida en la que esta el jugador (NULL si no esta en ninguna)
 *
 *  returns: void
 *
 */
void cb_who(char *args, Jugador** j, Partida** p){
	char resp[250];

	sprintf(resp,"socket:%d user:%s",(*j)->id,(*j)->username);
	send((*j)->id, resp,strlen(resp),0);

}

/**
 * Function: cb_usuario
 * --------------------
 *
 *  Condiciones previas:
 *  - Jugador conectado
 *  - Jugador no logeado
 *
 *	Comprueba que el nombre de usuario recibido en args esta en la base de datos
 *  si esta, se lo asigna al jugador a la espera de una PASSWORD que sera el
 *  siguiente comando que el jugador debe introducir para logearse.
 *
 *  char *args    argumentos del comando (NOMBRE DE USUARIO)
 *  Jugador** j   jugador al que responder
 *  Partida** p   (NULL) Antes de logearse no debe estar en ninguna partida
 *
 *  returns: void
 *
 */
void cb_usuario(char *args, Jugador** j, Partida** p){
	char resp[250];
	char* username = strtok(args, " \n");

	if(*j==NULL)
		return;
	else if( (*j)->logeado == 1 ){
		strcpy(resp,"-Err. Ya estas logeado.");
		send((*j)->id,resp,strlen(resp),0);
	}
	else if(jugador_registrado(username)){

		/* primer argumento recibido (username) a (*j)->username */
		strcpy( (*j)->username, username );

		strcpy(resp,"+Ok. Usuario correcto.");
		send((*j)->id,resp,strlen(resp),0);

	}
	else{
		strcpy(resp,"-ERR. Usuario incorrecto.\n");
		send((*j)->id,resp,strlen(resp),0);
	}
}


/**
 * Function: cb_password
 * --------------------
 *
 *  Condiciones previas:
 *  - Nombre de usuario del jugador distinto de DESCONOCIDO
 *  - Jugador no logeado
 *
 *	Comprueba que el nombre de usuario del jugador y la PASSWORD especificada
 *  en el argumento coinciden con la base de datos, si es asi, se marca jugador como
 *  logeado
 *
 *  char *args    argumentos del comando (PASSWORD)
 *  Jugador** j   jugador a logear y al que responder
 *  Partida** p   (NULL) Antes de logearse no debe estar en ninguna partida
 *
 *  returns: void
 *
 */
void cb_password(char *args, Jugador** j, Partida** p){
	char resp[250];
	char* password = strtok(args, " \n");

	if(*j==NULL){
		//no hacer nada
	}
	else if( (*j)->logeado == 1 ){
		strcpy(resp,"-Err. Ya estas logeado.");
		send((*j)->id,resp,strlen(resp),0);
	}
	else if(strcmp((*j)->username,"DESCONOCIDO")==0){
		strcpy(resp,"-Err. Debes especificar primero tu nombre de usuario.");
		send((*j)->id,resp,strlen(resp),0);
	}
	else if( jugador_login(j, password) > 0 ){

		strcpy(resp,"+Ok. Login correcto.");
		send((*j)->id,resp,strlen(resp),0);

	}
	else{
		strcpy(resp,"-ERR. Password incorrecta.");
		send((*j)->id,resp,strlen(resp),0);
	}
}

/**
 * Function: cb_register
 * --------------------
 *
 *  Condiciones previas:
 *  - Usuario conectado al servidor
 *  - Recibir args con formato -u  <username> -p <password>
 *  - <username> no debe existir en la base de datos
 *  - longitud de username menor de 40 caracteres
 *  - longitud de password menor de 128 caracteres
 *
 *	Registra en la base de datos un usuario nuevo.
 *
 *  char *args    argumentos del comando (-u  <username> -p <password>)
 *  Jugador** j   cliente que efectua el registro
 *  Partida** p   (NULL) No se necesita
 *
 *  returns: void
 *
 */
void cb_register(char *args, Jugador** j, Partida** p){
	char resp[250];
	char username[40];
	char password[128];
	char *pch;
	int error = 0;

	pch = strtok(args," ");
	if(strcmp(pch,"-u")==0){
		pch = strtok(NULL, " ");
		if(pch!=NULL && strlen(pch)<40){
			strcpy(username, pch);
			pch = strtok(NULL, " ");
			if(strcmp(pch,"-p")==0){
				pch = strtok(NULL, " \n");
				if(pch!=NULL && strlen(pch)<128){
					strcpy(pch,password);
				}
				else{
					/*error de formato*/
					error=1;
				}
			}
			else{
				error=1;
			}
		}
		else{
			error=1;
		}

	}
	else{
		error=1;
	}

	if(error==1){
		strcpy(resp,"-ERR. Error de formato: REGISTER -u <username> -p <password>");
		send((*j)->id,resp,strlen(resp),0);
	}
	else if(jugador_registrar(username, password)>0){
		strcpy(resp,"+Ok. Usuario registrado con exito.");
		send((*j)->id,resp,strlen(resp),0);
	}
	else{
		/*usuario ya registrado.*/
		strcpy(resp,"+Err. Usuario ya existente.");
		send((*j)->id,resp,strlen(resp),0);
	}
}