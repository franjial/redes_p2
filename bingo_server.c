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

	Command* cmd_cli = NULL;




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




	cmd_ini(&cmd_cli);

	/*registro de comandos que acepto de los clientes*/
	cmd_reg(&cmd_cli, "WHO", &cb_who);
	cmd_reg(&cmd_cli, "USUARIO", &cb_usuario);
	cmd_reg(&cmd_cli, "PASSWORD", &cb_password);
	cmd_reg(&cmd_cli, "REGISTER", &cb_register);
	cmd_reg(&cmd_cli, "INICIAR-PARTIDA", &cb_iniciar_partida);
	cmd_reg(&cmd_cli, "CARTON", &cb_carton);
	cmd_reg(&cmd_cli, "PARTIDA", &cb_partida);
	cmd_reg(&cmd_cli, "SALIR", &cb_salir);
	cmd_reg(&cmd_cli, "BINGO", &cb_bingo);
	cmd_reg(&cmd_cli, "UNA-LINEA", &cb_linea);
	cmd_reg(&cmd_cli, "DOS-LINEAS", &cb_slinea);

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
		alarm(1);
		if(sacar==1){
			signal(SIGALRM, SIG_IGN);

			/*sacar bolas*/
			for(i=0;i<10;i++){
				/*reviso todas las partidas*/

				if(partida[i]!=NULL){
					if(partida[i]->ganador_bingo!=NULL){
						/*si tiene ganador*/
						for(j=0;j<4;j++){
							bzero(buffer,sizeof(buffer));
							sprintf(buffer,"+Ok. Fin partida. Ganada por %s.\n",partida[i]->ganador_bingo->username);
							if(partida[i]->jugadores[j]!=NULL)
								send(partida[i]->jugadores[j]->id,buffer,strlen(buffer),0);
						}
						partida_clean(&partida[i]);
						partida[i]=NULL;
					}
					else if(partida[i]->bombo == NULL){
						/*partida terminada sin ganador*/

						for(j=0;j<4;j++){
							bzero(buffer,sizeof(buffer));
							sprintf(buffer,"+Ok. Partida terminada.\n");
							if(partida[i]->jugadores[j]!=NULL)
								send(partida[i]->jugadores[j]->id,buffer,strlen(buffer),0);
						}

						partida_clean(&partida[i]);
						partida[i]=NULL;
					}
					else if(partida[i]->njugadores==0){
						/*salieron todos los jugadores.*/
						partida_clean(&partida[i]);
						partida[i]=NULL;

					}

					else if(partida[i]->iniciada == 1){
						/*partida iniciada, sacar bola del bombo*/
						bola = partida_sacar(&partida[i]);
						if(bola > 0){
							for(j=0;j<4;j++){
								if(partida[i]->jugadores[j]!=NULL){
									bzero(buffer,sizeof(buffer));
									sprintf(buffer,"\nEl %d!\n",bola);
									send(partida[i]->jugadores[j]->id,buffer,strlen(buffer),0);
								}
							}
						}

					}

					else{
						/*partida no iniciada, comprobar si esta llena y marcar para inicio*/
						if(partida[i]->njugadores == 4){
							partida[i]->iniciada=1;
							for(j=0;j<4;j++){
								strcpy(buffer,"+Ok. Comenzando partida!");
								send(partida[i]->jugadores[j]->id,buffer, strlen(buffer), 0);
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

								/*le asigno al nuevo jugador su numero de socket y slot*/
								jugador[ret]->id = new_sd;
								jugador[ret]->slot = ret;

								FD_SET(new_sd, &readfds);
								strcpy(buffer,"+Ok. Usuario conectado");
								send(new_sd, buffer, strlen(buffer),0);

							}

						}

					}
					else if(i==0){ /*se ha introducido info. teclado*/
						bzero(buffer, sizeof(buffer));
						fgets(buffer, sizeof(buffer), stdin);


						/*cerrar todos los clientes y cerrar servidor*/
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
						else{
							printf("-Err. Comando desconocido.\n");
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
								cmd_exe(cmd_cli, buffer, &jugador[ret], NULL);
							}
							else
								cmd_exe(cmd_cli, buffer, &jugador[ret], &partida[jugador[ret]->id_partida]);

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
 * Function: cb_buscar_partida
 * --------------------
 *
 *  Condiciones previas:
 *  - Jugador logeado
 *  - Jugador sin partida
 *
 *	Une un jugador a una partida
 *
 *  Recibe:
 *  Jugador** jugador    jugador al que se le asigna partida
 *
 *  modifica: (*jugador)->id_partida con nueva partida asignada
 *  devuelve: slot de partida asignada
 *            -1 si no se pudo asignar partida
 *
 */
int buscar_partida(Jugador** jugador){
	int i;

	/*primero buscamos salas con gente QUE NO ESTEN INICIADAS*/
	for(i=0;i<10;i++){
		if(partida[i]!=NULL){
			if( partida[i]->njugadores < 4  && partida[i]->iniciada == 0){
				partida_ingresar(&partida[i], jugador);
				return i;
			}
		}
	}

	/*despues buscamos salas vacias*/
	for(i=0;i<10;i++){
		if(partida[i]==NULL){
			partida_nueva(i, &partida[i]);
			partida_ingresar(&partida[i], jugador);
			return i;
		}
	}

	/*si llegamos aqui esta completo*/
	return -1;
}

/**
 * Nuevo jugador en el sistema.
 */
int ingresar_jugador(Jugador* jugador[40]){
	int i;

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
		if(j[i]!=NULL && j[i]->id == id){
			return i;
		}
	}
	return -1;
}


/**
 * cmd_ini inicializa la lista enlazada de comandos
 */
void cmd_ini(Command** head){

	(*head) = NULL;
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



	id = strtok(buffer," \n"); /*extraer comando*/
	args = strtok(NULL, "\n");

	printf("%s|%s\n",id,args);


	while(cmd_head!=NULL && strcmp(cmd_head->buffer,id)!=0){
		/* pone cmd_head al nodo que coincide con comando dentro de buffer,
	   * o al final de la lista. */
		cmd_head = cmd_head->next;
	}

	if(cmd_head != NULL){
		/*pongo en buffer los argumentos (puede no tener argumentos)*/
		if(args!=NULL){

			printf("DEBUG: %s\n",args);
			/*ejecutar funcion indicada en el nodo*/
			cmd_head->cb(args,j,p);
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

	sprintf(resp,"slot: %d socket:%d user:%s partida:%d",(*j)->slot,(*j)->id,(*j)->username,(*j)->id_partida);
	send((*j)->id, resp,strlen(resp),0);

}

/**
 * Function: cb_usuario
 * --------------------
 *
 *  Condiciones previas:
 *  - Jugador conectado
 *  - Jugador no logeado
 *  - Recibir un argumento (username)
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
	else if(username==NULL){
		strcpy(resp,"-ERR. Nombre de usuario incorrecto");
		send((*j)->id,resp,strlen(resp),0);
	}
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
 *  - Recibir un argumento
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
	else if(password==NULL){
		strcpy(resp,"-ERR. Password incorrecta.");
		send((*j)->id,resp,strlen(resp),0);
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
	printf("-u[%s]\n",pch);
	if(strcmp(pch,"-u")==0){
		pch = strtok(NULL, " ");
		if(pch!=NULL && strlen(pch)<40){
			strcpy(username, pch);
			pch = strtok(NULL, " ");
			printf("-p[%s]\n",pch);
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


/**
 * Function: cb_iniciar_partida
 * --------------------
 *
 *  Condiciones previas:
 *  - No estar ya en una partida
 *  - Estar logeado
 *  - Requiere array completo de partidas
 *
 *	Se le asigna al jugador una partida
 *
 *  char *args    (NULL) No recibe parametros
 *  Jugador** j   jugador
 *  Partida** p   (NULL) No se necesita
 *
 *  returns: void
 *
 */
void cb_iniciar_partida(char *args, Jugador** j, Partida** p){
	char resp[250];
	int asignada;

	if((*j)->logeado==0 || (*j)->id_partida>-1){
		strcpy(resp,"-ERR. Debes estar logeado y no tener partida.");
		send((*j)->id,resp,strlen(resp),0);
	}else{
		asignada = buscar_partida(j);
		if(asignada!=-1)
			sprintf(resp,"+Ok. Ahora estas en partida %d.",asignada);
		else
			sprintf(resp,"-Err. Todas las partidas estan llenas.");

		send((*j)->id,resp,strlen(resp),0);
	}
}


/**
 * Function: cb_carton
 * --------------------
 *
 *  Condiciones previas:
 *  - Usuario identificado
 *  - Usuario conectado a partida
 *
 *	Muestra carton con el que se esta jugando o se va a jugar.
 *
 *  char *args    argumentos del comando (NULL)
 *  Jugador** j   jugador
 *  Partida** p   (NULL) No se necesita
 *
 *  returns: void
 *
 */
void cb_carton(char *args, Jugador**j, Partida **p){
	char resp[250];

	if((*j)->logeado==0 || (*j)->id_partida==-1){
		strcpy(resp,"-ERR. Debes estar conectado y en una partida.");
		send((*j)->id,resp,strlen(resp),0);
	}else{
		carton_str(resp,(*j)->carton);
		send((*j)->id,resp,strlen(resp),0);
	}

}

/**
 * Function: cb_partida
 * --------------------
 *
 *  Condiciones previas:
 *  - Usuario identificado
 *  - Usuario conectado a partida
 *
 *	Muestra jugadores dentro de partida
 *
 *  char *args    argumentos del comando (NULL)
 *  Jugador** j   jugador
 *  Partida** p   partida donde esta el jugador
 *
 *  returns: void
 *
 */
void cb_partida(char *args, Jugador **j, Partida **p){
	char resp[250];

	if(j == NULL){
		/*no tengo jugador, no puedo responder con nada*/
		return;
	}
	else if(p == NULL){
		strcpy(resp,"-ERR. Debes estar en una partida.");
	}
	else{
		if((*j)->logeado==0 || (*j)->id_partida==-1){
			strcpy(resp,"-ERR. Debes estar logeado y en una partida.");
		}
		else{
			printf("PARTIDA_DEBUG: %s:%d\n",(*j)->username,(*p)->id);
			partida_jugadores_str(*p, resp);

		}
	}

	send((*j)->id,resp,strlen(resp),0);


}


/**
 * Function: cb_salir
 * --------------------
 *
 *  Condiciones previas:
 *  - Usuario conectado
 *
 *	Saca jugador de partida si esta en una, e informa a jugadores de su partida
 *  Saca jugador de su slot.
 *
 *  char *args    argumentos del comando (NULL)
 *  Jugador** j   jugador
 *  Partida** p   partida donde esta el jugador
 *
 *  returns: void
 *
 */
void cb_salir(char *args, Jugador**j, Partida **p){
	partida_sacar_jugador(p,(*j)->id);
	jugador[(*j)->slot] = NULL;
	free(*j);
}

/**
 * Function: cb_bingo
 * --------------------
 *
 *  Condiciones previas:
 *  - Usuario en una partida iniciada
 *
 *	Jugador canta bingo, comprobamos que es verdad y
 *  si es asi, terminamos la partida. Si no tiene bingo,
 *  se lo comunicamos. Si termina la partida, sacamos
 *  a todos los jugadores.
 *
 *  char *args    argumentos del comando (NULL)
 *  Jugador** j   jugador
 *  Partida** p   partida donde esta el jugador
 *
 *  returns: void
 *
 */
void cb_bingo(char *args, Jugador**j, Partida **p){
	char resp[250];
	int i;
	if(*j == NULL){
		strcpy(resp,"-Err. No se quien eres.\n");
		send((*j)->id,resp,strlen(resp),0);
	}
	else if(*p==NULL){
		strcpy(resp,"-Err. No estas en partida.");
		send((*j)->id,resp,strlen(resp),0);
	}
	else if((*p)->iniciada == 0){
		strcpy(resp,"-Err. No procede.");
		send((*j)->id,resp,strlen(resp),0);
	}
	else{
		if( carton_bingo((*j)->carton, (*p)->fuera) ){
			/*hay bingo*/
			sprintf(resp,"Bingo! Ha ganado %s.",(*j)->username);
			for(i=0;i<4;i++){
				send((*p)->jugadores[i]->id,resp,strlen(resp),0);
			}
			strcpy(resp,"+Ok. Has ganado.\n");
			send((*j)->id,resp,strlen(resp),0);

			(*p)->ganador_bingo = *j;

		}
		else{
			/*no hay bingo*/
			strcpy(resp,"-Err. No procede.");
			send((*j)->id,resp,strlen(resp),0);
		}
	}
}


/**
 * Function: cb_linea
 * --------------------
 *
 *  Condiciones previas:
 *  - Usuario en una partida iniciada
 *
 *	Jugador canta linea, comprobamos que es verdad
 *  si es asi marcamos linea. Si no tiene linea,
 *  se lo comunicamos.
 *
 *  char *args    argumentos del comando (NULL)
 *  Jugador** j   jugador
 *  Partida** p   partida donde esta el jugador
 *
 *  returns: void
 *
 */


void cb_linea(char *args, Jugador**j, Partida **p){

}



/**
 * Function: cb_slinea
 * --------------------
 *
 *  Condiciones previas:
 *  - Usuario en una partida iniciada
 *
 *	Jugador canta segunda linea, comprobamos que es verdad
 *  si es asi marcamos segunda linea. Si no tiene linea,
 *  se lo comunicamos.
 *
 *  char *args    argumentos del comando (NULL)
 *  Jugador** j   jugador
 *  Partida** p   partida donde esta el jugador
 *
 *  returns: void
 *
 */
void cb_slinea(char *args, Jugador**j, Partida **p){

}




/**
 * COMANDOS ACEPTADOS POR EL SERVIDOR
 */

/**
 * Function: cb_cerrar
 * --------------------
 *
 *	Envia mensaje de cierre a todos los clientes conectados
 *
 *  char *args    argumentos del comando (NULL)
 *  Jugador** j   NULL
 *  Partida** p   NULL
 *
 *  returns: void
 *
 */
void cb_cerrar(char *args, Jugador**j, Partida **p){

}

/**
 * Function: cb_clients
 * --------------------
 *
 *	Imprime numero de clientes conectados
 *
 *  char *args    argumentos del comando (NULL)
 *  Jugador** j   NULL
 *  Partida** p   NULL
 *
 *  returns: void
 *
 */
void cb_clients(char *args, Jugador**j, Partida **p){
	int i;
	int count = 0;

	for(i=0;i<40;i++){
		if(jugador[i]!=NULL)
			count++;
	}

	printf("+Ok. Hay %d clientes conectados.\n");
}
