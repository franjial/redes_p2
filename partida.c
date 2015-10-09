#include <stdio.h>
#include <stdlib.h>
#include "cartones.h"
#include "partida.h"
#include "bolas.h"


int partida_nueva(int id, Partida** partida){
	

	*partida = (Partida *) malloc(sizeof(Partida));
	(*partida)->iniciada = 0;

	bombo_gen( &((*partida)->bombo), 90, 1);
	(*partida)->fuera = NULL;
	(*partida)->id = id; 
	(*partida)->njugadores = 0;

}

int partida_sacar(Partida** partida){
	return bombo_pop(&(*partida)->bombo, NULL);
}

int partida_bingo(Partida** partida, Jugador* jugador){
	(*partida)->ganador_bingo = jugador;
	return 1;
}

int partida_linea(Partida** partida, Jugador* jugador){
	(*partida)->ganador_linea = jugador;
	return 1;
}

int partida_slinea(Partida** partida, Jugador* jugador){
	(*partida)->ganador_slinea = jugador;
	return 1;
}

int partida_ingresar(Partida** partida, Jugador** jugador){
	if((*partida)->njugadores>=4)
		return -1;
	carton_nuevo((*jugador)->carton);
	(*partida)->jugadores[(*partida)->njugadores] = *jugador;
	(*partida)->njugadores++;
	return (*partida)->njugadores;

}


/**
 * Libera memoria tras terminar una partida y saca
 * a los jugadores. Le asigna un nuevo carton aleatorio
 * a cada uno.
 */
int partida_clean(Partida** partida){
	int i,j;

	bombo_clean( &(*partida)->bombo ); //limpiar bombo 
	bombo_clean( &(*partida)->fuera ); //limpiar bombo bolas sacadas

	for(i=0;i<4;i++){
		(*partida)->jugadores[i]->listo = 0;
		(*partida)->jugadores[i]->id_partida = -1;
		carton_nuevo((*partida)->jugadores[i]->carton);
		(*partida)->jugadores[i] = NULL;
	}

	free(*partida);

}

/** 
 * Sacar jugador de una partida si esta dentro de ella
 */
int partida_sacar_jugador(Partida** partida, int id_jugador){
	int i;

	if(*partida == NULL){
		return 0;
	}

	for(i=0;i<4;i++){
		if((*partida)->jugadores[i]->id==id_jugador){
			(*partida)->jugadores[i]=NULL;
			return 1;
		}
	}
	return 0;
}