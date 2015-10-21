#ifndef BINGO_SERVER_H
#define BINGO_SERVER_H

#include "partida.h"
#include "jugador.h"


#define MSG_SIZE 250
#define MAX_CLIENTS 40
#define MAX_PARTIDAS 10


static int sacar = 0; /*si esta a uno se saca bola en partidas*/
static int salir = 0;

/**
 * - Maximo 4 jugadores/partida
 * - Maximo 10 partidas
 * - Maximo 1 carton por jugador
 */

typedef
void (*cmd_func_t)(char *buffer, Jugador** j, Partida** p);

typedef
struct command{
	cmd_func_t cb; /*callback*/
	char buffer[250];
	struct command *next;
}Command;


void manejador(int signum);
void sacar_bolas(int signum);

int buscar_partida(Partida* partida[10], Jugador* jugador);
int ingresar_jugador(Jugador* jugador[40]);
int buscar_jugador(Jugador* j[],int id);


void cmd_ini(Command** cmd_head);
void cmd_reg(Command** cmd_head, char *buffer, void (*cb)(char *buffer, Jugador** j, Partida** p));
int cmd_exe(Command* cmd_head, char *buffer, Jugador** j, Partida** p);
void cmd_clean(Command** cmd_head);


/* comandos disponibles */
void cb_who(char *args, Jugador** j, Partida** p);
void cb_usuario(char *args, Jugador** j, Partida** p);
void cb_password(char *args, Jugador** j, Partida** p);

#endif
