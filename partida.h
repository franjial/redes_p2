#ifndef PARTIDA_H
#define PARTIDA_H

#include "jugador.h"
#include "bolas.h"

typedef
struct partida{
	int id;
	int njugadores;
	Jugador* jugadores[4];
	Jugador* ganador_bingo;
	Jugador* ganador_linea;
	Jugador* ganador_slinea;
	Bola* fuera; /*bolas fuera del bombo*/
	Bola* bombo; /*bolas dentro del bombo*/
	int iniciada;
}Partida;


int partida_nueva(int id, Partida** partida);
int partida_sacar(Partida** partida); /*sacar bola*/

int partida_bingo(Partida* partida, Jugador* jugador); /*comprueba bingo*/
int partida_linea(Partida* partida, Jugador* jugador); /*comprueba linea*/
int partida_slinea(Partida* partida, Jugador* jugador); /*comprueba segunda linea*/

int partida_ingresar(Partida** partida, Jugador** jugador); /*ingresa un jugador*/
int partida_clean(Partida** partida);

int partida_sacar_jugador(Partida** partida, int id_jugador); /*sacar jugador de la partida*/

void partida_jugadores_str(Partida *partida, char buffer[250]);
#endif