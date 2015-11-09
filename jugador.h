#ifndef JUGADOR_H
#define JUGADOR_H

#include "cartones.h"

typedef
struct jugador{
	int id; /*descriptor socket*/
	int slot; /*posicion en array de jugadores*/
	char username[40];
	int id_partida; /*id partida asignada, -1 si en ninguna*/
	int listo; /*bool true si jugador listo para empezar*/
	int logeado; /*bool true si esta logeado correctamente*/

	int carton[3][9];

}Jugador;



int jugador_nuevo(Jugador** jugador,int slot);

int jugador_registrar(const char* username, const char* pass);
int jugador_login(Jugador* j, const char pass[128]);
int jugador_registrado(const char username[40]);

#endif
