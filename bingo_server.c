#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "partida.h"
#include "jugador.h"


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <signal.h>


#define MSG_SIZE 250
#define MAX_CLIENTS 40
#define MAX_PARTIDAS 10

static int sacar = 0; /*si esta a uno se saca bola en partidas*/

/**
 * - Maximo 4 jugadores/partida
 * - Maximo 10 partidas
 * - Maximo 1 carton por jugador
 */

void manejador(int signum);
void sacar_bolas(int signum);

int buscar_partida(Partida* partida[10], Jugador* jugador);
int ingresar_jugador(Jugador* jugador[40]);
int buscar_jugador(Jugador* j[],int id);

int
main(int argc, char* argv[]){
	int i,j;

	Partida* partida[10];
	Jugador* jugador[40];
	
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
			}

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
								printf("Nuevo jugador id %d en slot %d \n",new_sd,ret);
								FD_SET(new_sd, &readfds);
								strcpy(buffer,"+Ok. Usuario conectado");
								send(new_sd, buffer, strlen(buffer),0);

							}

						}

					}
					else if(i==0){ /*se ha introducido info. teclado*/
						bzero(buffer, sizeof(buffer));
						fgets(buffer, sizeof(buffer), stdin);

						if(strcmp(buffer,"SALIR\n") == 0){
							// enviar a todos los clientes salir

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
							pch = strtok(buffer," \n"); /*primera llamada a strtok contiene comando*/
							printf("%s",buffer);
							if(strcmp(pch,"USUARIO")==0){
								ret=buscar_jugador(jugador,i);

								if(ret!=-1 && jugador[ret]!=NULL){
									bzero(jugador[ret]->username, sizeof(jugador[ret]->username));
									bzero(identificador, sizeof(identificador));

									pch = strtok(NULL," \n"); /*segunda llamada a strtock pch contiene primer argumento*/

									/*comprobar que el username esta registradon el el fichero*/
									if(jugador_registrado(pch)==1){

										/*copio cadena despues del primer espacio en username->jugador*/
										strcpy(jugador[ret]->username, pch);
										/*sprintf(identificador,"slot:%d socket:%d #%s#",ret,jugador[ret]->id,jugador[ret]->username);*/
										strcpy(buffer,"+Ok. Usuario correcto.");
										send(i,buffer,strlen(buffer),0);
									}
									else{
										strcpy(buffer,"-ERR. Usuario incorrecto.\n");
										send(i,buffer,strlen(buffer),0);
									}

								}
							}

							/*si recibo password y es correcta, hago login del usuario*/
							if(strcmp(pch,"PASSWORD")==0){
								ret=buscar_jugador(jugador,i);
								if(ret!=-1 && jugador[ret]!=NULL){
									pch = strtok(NULL," \n"); /*segunda llamada a strtock pch contiene primer argumento*/

									/*intento login, informar del resultado*/
									if(jugador_login(&jugador[ret],pch) > 0){
										sprintf(buffer,"+Ok. Se ha logeado como %s",jugador[ret]->username);
										printf("Error pass #%s# por %d\n",pch,i);
										send(i,buffer,strlen(buffer),0);
									}
									else{
										printf("ERROR #%s# por %d\n",pch,i);
										strcpy(buffer,"-ERR. Contraseña incorrecta.");
										send(i,buffer,strlen(buffer),0);
									}

								}
							}

							/*registrar usuario*/
							if(strcmp(pch,"REGISTER")==0){
								if(strcmp(strtok(NULL," "),"-u")==0){
									strcpy(reg_username, strtok(NULL, " "));
									if(strcmp(strtok(NULL, " "),"-p")==0){
										strcpy(reg_pass, strtok(NULL," \n"));
										if(jugador_registrar(reg_username, reg_pass)>0){
											strcpy(buffer,"+Ok. Usuario registrado correctamente");
											send(i,buffer,strlen(buffer),0);
										}
										else{
											strcpy(buffer,"-Err. No se pudo registrar el usuario");
											send(i,buffer,strlen(buffer),0);
										}
									}
									else{
										/*error*/
										strcpy(buffer,"-Err. Comando desconocido");
										send(i,buffer,strlen(buffer),0);
									}
								}
								else{
									/*error*/
									strcpy(buffer,"-Err. Comando desconocido");
									send(i,buffer,strlen(buffer),0);
								}

								bzero(reg_username,sizeof(reg_username));
								bzero(reg_pass,sizeof(reg_pass));
							}

							if(strcmp(pch,"INICIAR-PARTIDA")==0){

								bzero(buffer,sizeof(buffer));

								ret=buscar_jugador(jugador,i);
								if(ret!=-1){
									if(jugador[ret]!=NULL){

										jugador[ret]->id_partida = buscar_partida(partida,jugador[ret]);
										if(jugador[ret]->id_partida == -1){
											/*no se pudo asignar partida: mandar mensaje de error.*/

											strcpy(buffer,"-ERR. No hay partidas disponibles.");
											send(i,buffer,strlen(buffer),0);
										}
										else{
											printf("Intentado iniciar partida %d",jugador[ret]->id_partida);
											jugador[ret]->listo = 1;
											/*Si estan los cuatro jugadores listos, marcar para iniciar*/
											if(partida[jugador[ret]->id_partida]->njugadores == 4){
												todos_listos=1;
												for(j=0;j<4;j++){
													if(partida[jugador[ret]->id_partida]->jugadores[j]->listo == 0){
														todos_listos=0;
														break;
													}
												}
												if(todos_listos==0){
													/*no estan todos listos, informar*/
													sprintf(buffer,"+Ok. Peticion Recibida. Quedamos a la espera de mas jugadores");
													send(i,buffer,strlen(buffer),0);
												}
												else{
													/*estan todos listos, informar y marcar para iniciar*/

													partida[jugador[ret]->id_partida]->iniciada = 1;
													printf("inciando partida");
													strcpy(buffer,"+Ok. Empieza la partida");
													for(j=0;j<4;j++){ /*a todos los miembros de la partida*/
														send(partida[jugador[ret]->id_partida]->jugadores[j]->id,
																 buffer,strlen(buffer),0);
													}
												}
											}
											else{
												/*no estan todos*/
												strcpy(buffer,"+Ok. Peticion Recibida. Quedamos a la espera de mas jugadores");
												send(i,buffer,strlen(buffer),0);

											}
										}

									}
								}
								else{
									/*error no esta conectado*/
									strcpy(buffer,"-Err. No estas conectado.");
									send(i,buffer,strlen(buffer),0);
								}
							}

							if(strcmp(buffer,"ID")==0){
								bzero(identificador,sizeof(identificador));
								sprintf(identificador,"%d",i);
								send(i,identificador,strlen(identificador),0);
							}

							if(strcmp(buffer,"WHO")==0){
								ret = buscar_jugador(jugador,i);
								sprintf(identificador,"slot:%d socket:%d user:%s",ret,i,jugador[ret]->username);
								if(ret!=-1 && jugador[ret]!=NULL){
											//send(i,jugador[ret]->username,strlen(jugador[ret]->username),0);
									send(i,identificador,strlen(identificador),0);

								}
								else{
									send(i,"DESCONOCIDO",11,0);
								}
							}

							if(strcmp(buffer,"PARTIDA")==0){
								bzero(buffer,sizeof(buffer));

								ret=buscar_jugador(jugador,i);
								if(ret!=-1){
									if(jugador[ret]->id_partida != -1){

										for(j=0;j<4;j++){
											if(partida[jugador[ret]->id_partida]->jugadores[j]!=NULL){
												sprintf(buffer, "\n%d\t%s\t%d",j, partida[jugador[ret]->id_partida]->jugadores[j]->username,
																										 partida[jugador[ret]->id_partida]->jugadores[j]->listo);

												send(i,buffer,strlen(buffer),0);
											}
											else{
												sprintf(buffer, "\n%d\tVACIO",j);
												send(i,buffer,strlen(buffer),0);
											}
										}



									}else{
										strcpy(buffer,"-ERR. No estas en ninguna partida");
										send(i,buffer,strlen(buffer),0);
									}
								}else{
									strcpy(buffer,"-ERR. No estas conectado");
									send(i,buffer,strlen(buffer),0);
								}

							}
						}
					}
			}
		}

	}

}



return EXIT_SUCCESS;
}




void manejador(int signum){
	/*indicar a clientes que se cierren*/
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

	/*primero buscamos salas con gente*/
	for(i=0;i<10;i++){
		if(partida[i]!=NULL){
			if( partida[i]->njugadores < 4 ){
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