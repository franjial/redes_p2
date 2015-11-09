#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cartones.h"
#include "partida.h"
#include "bolas.h"


void partida_nueva(int id, Partida** partida){
	int i;



	*partida = (Partida *) malloc(sizeof(Partida));
	(*partida)->iniciada = 0;
	(*partida)->terminada = 0;
	(*partida)->linea = 0;
	(*partida)->slinea = 0;

	bombo_gen( &((*partida)->bombo), 90, 1);

	(*partida)->id = id;
	(*partida)->njugadores = 0;

	/*inicializar punteros de jugadores a null*/
	for(i=0;i<4;i++){
		(*partida)->jugadores[i] = NULL;
	}

	for(i=0;i<90;i++){
		(*partida)->fuera[i]=0;
	}

	for(i=0;i<4;i++){
		(*partida)->ganador_bingo[i] = NULL;
		(*partida)->ganador_linea[i] = NULL;
		(*partida)->ganador_slinea[i] = NULL;
	}

}


/**
 * Saca una bola del bombo de la partida y la devuelve.
 * Si se produce algun error, devuelve -1.
 */
int partida_sacar(Partida* partida){
	int i;
	i = bombo_pop(&partida->bombo);

	if(i>0 && i<90){
		partida->fuera[i-1]=1;
		return i;
	}
	else{
		return -1;
	}
}



/**
 * partida_ingresar(Partida**, Jugador **)
 *
 * ingresa un jugador en una partida y le asigna un carton
 * Modifica:
 *   Jugador -> se le asigna nuevo carton
 *              se le asigna id_partida
 *
 *   Partida -> nuevo jugador en posicion vacia de Partida->jugadores
 *              se incrementa Partida->njugadores
 *
 * Devuelve:
 *   Si se logra ingresar al nuevo jugador devuelve numero de jugadores en partida
 *   en caso contrario devuelve -1
 *
 */
int partida_ingresar(Partida* p, Jugador* j){
	int i;

	if(p->njugadores >= 4)
		return -1;

	/*buscar una posicion vacia en la partida para el nuevo participante*/
	for(i=0;i<4;i++){
		if( p->jugadores[i] == NULL ){

			carton_nuevo(j->carton);
			j->id_partida = p->id;

			p->jugadores[i] = j;
			p->njugadores++;

			return p->njugadores;
		}
	}


	return -1; /*no se pudo asignar, partida llena*/

}

/**
 * Libera memoria tras terminar una partida y saca
 * a los jugadores. Le asigna un nuevo carton aleatorio
 * a cada uno.
 */
void partida_clean(Partida** p){
	int i,j;

	bombo_clean( &(*p)->bombo ); //limpiar bombo

	for(i=0;i<4;i++){
		if((*p)->jugadores[i]!=NULL){
			(*p)->jugadores[i]->id_partida = -1;
			(*p)->jugadores[i] = NULL;
		}
	}

	free(*p);

}

/**
 * Sacar jugador de una partida si esta dentro de ella
 * Modifica: jugador con id_jugador dentro de partida
 *           njugadores: si saca un jugador de partida actualiza valor de njugadores
 */
int partida_sacar_jugador(Partida* partida, int id_jugador){
	int i;

	if(partida == NULL){
		return 0;
	}


	for(i=0;i<4;i++){
		if( partida->jugadores[i] != NULL ){
			if( partida->jugadores[i]->id == id_jugador ){
				partida->jugadores[i]->id_partida = -1;
				partida->jugadores[i]=NULL;
				partida->njugadores--;
				return 1;
			}
		}

	}

	return 0;
}

/**
 * Escribe en buffer, un listado con los jugadores dentro de la partida
 */
void partida_jugadores_str(Partida *partida, char buffer[250]){
	int i;
	char aux[55];

	bzero(buffer,sizeof(buffer));

	if( partida == NULL )
		return;

	for(i=0;i<4;i++){
		if(partida->jugadores[i]!=NULL){
			bzero(aux,sizeof(aux));
			sprintf(aux,"[%d:%s] ",i,partida->jugadores[i]->username);
			strcat(buffer,aux);
		}
		else{
			bzero(aux,sizeof(aux));
			sprintf(aux,"[%d:VACIO] ",i);
			strcat(buffer,aux);
		}
	}

}

/**
 * inserta en partida un jugador que gana bingo
 * si ya esta insertado, no hace nada
 */
void partida_bingo(Partida* partida, Jugador* j){
	int i;

	for(i=0;i<4;i++){
		if(partida->ganador_bingo[i]==j)
			return;
	}


	i=0;
	while(partida->ganador_bingo[i]!=NULL && i<4)
		i++;

	partida->ganador_bingo[i] = j;

}

/**
 * inserta en partida un jugador que gana linea
 * si ya esta insertado, no hace nada
 */
void partida_linea(Partida* partida, Jugador* j){
	int i = 0;

	/*comp. si esta ya*/
	for(i=0;i<4;i++){
		if(partida->ganador_linea[i]==j)
			return;
	}

	i=0;
	while(partida->ganador_linea[i]!=NULL && i<4)
		i++;

	partida->ganador_linea[i] = j;

}

/**
 * inserta en partida un jugador que gana segunda linea
 * si ya esta insertado, no hace nada
 */
void partida_slinea(Partida* partida, Jugador* j){
	int i = 0;

	/*comp. si esta ya*/
	for(i=0;i<4;i++){
		if(partida->ganador_slinea[i]==j)
			return;
	}


	i=0;
	while(partida->ganador_slinea[i]!=NULL && i<4)
		i++;

	partida->ganador_slinea[i] = j;

}