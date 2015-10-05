#ifndef JUGADOR_H
#define JUGADOR_H

#include "cartones.h"

typedef
struct jugador{
	int id;
	int carton[3][9];
	char username[40];
	char pass[128];
	int id_partida; /*id partida asignada, -1 si en ninguna*/
	int listo; /*bool true si jugador listo para empezar*/
	int logeado; /*bool true si esta logeado correctamente*/

}Jugador;



int jugador_nuevo(Jugador** jugador);
int jugador_logout(Jugador* jug);
int jugador_bingo(const Jugador* jug);
int jugador_linea(const Jugador* jug);
int jugador_slinea(const Jugador* jug);

int jugador_registrar(Jugador** j);
int jugador_login(Jugador** j);

#endif