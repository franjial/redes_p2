#include "jugador.h"
#include "cartones.h"
#include <stdlib.h>
#include <string.h>

/**
 * Crea un nuevo jugador. Si las credenciales son incorrectas no lo crea
 * y devuelve 0.
 */
int jugador_nuevo(Jugador** jugador){
	*jugador = (Jugador*) malloc(sizeof(Jugador));
	(*jugador)->id_partida = -1; /*ninguna partida asignada inicialmente*/
	strcpy((*jugador)->username,"DESCONOCIDO"); /*inicialmente desconocido*/
	strcpy((*jugador)->pass,""); /*inicialmente vacia*/

	return 1;
}
int jugador_logout(Jugador* jug){}
int jugador_bingo(const Jugador* jug){}
int jugador_linea(const Jugador* jug){}
int jugador_slinea(const Jugador* jug){}

/**
 * Registra un usuario. Si existe alguien con el username indicado
 * devuelve 0. Si se registra bien, devuelve 1. Lo marca como logeado
 * si se registra bien.
 */
int jugador_registrar(Jugador** j){

}

/**
 * Logea un jugador. Busca en un fichero un jugador, comprueba credenciales
 * y lo marca como logeado. Si hay error devuelve 0.
 */
int jugador_login(Jugador** j){

}