#ifndef JUGADOR_H
#define JUGADOR_H

#include "cartones.h"

typedef
struct jugador{
	int id; /*descriptor socket*/
	int slot; /*posicion en array de jugadores*/
	int carton[3][9];
	char username[40];
	int id_partida; /*id partida asignada, -1 si en ninguna*/
	int listo; /*bool true si jugador listo para empezar*/
	int logeado; /*bool true si esta logeado correctamente*/

}Jugador;



int jugador_nuevo(Jugador** jug);


int jugador_bingo(Jugador* jug);
int jugador_linea(Jugador* jug);
int jugador_slinea(Jugador* jug);

int jugador_registrar(const char* username, const char* pass);
int jugador_login(Jugador* jug, const char pass[128]);
int jugador_logout(Jugador* jug);
int jugador_registrado(const char username[40]);

#endif