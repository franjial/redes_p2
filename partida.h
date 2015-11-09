#ifndef PARTIDA_H
#define PARTIDA_H

#include "jugador.h"
#include "bolas.h"

typedef
struct partida{
	int id; /*posicion en el array de la partida*/
	int njugadores;
	int iniciada;

	Jugador* jugadores[4];
	Jugador* ganador_bingo;
	Jugador* ganador_linea;
	Jugador* ganador_slinea;
	int fuera[90]; /*bolas fuera del bombo*/
	Bola* bombo; /*bolas dentro del bombo*/

}Partida;


void partida_nueva(int id, Partida** partida);
int partida_sacar(Partida* partida); /*sacar bola*/


int partida_ingresar(Partida* p, Jugador* j); /*ingresa un jugador*/

void partida_clean(Partida** partida);

int partida_sacar_jugador(Partida** partida, int id_jugador); /*sacar jugador de la partida*/

void partida_jugadores_str(Partida *partida, char buffer[250]);
#endif
