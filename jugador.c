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
 * si se registra bien. Si error devuelve un codigo menor que 0.
 */
int jugador_registrar(Jugador** j){
	FILE *fp;
	char* linea = NULL;
	char* pch;
	int len = 100;
	linea = (char *)malloc(len+1);

	if( (fp=fopen("users.txt","r+")) == NULL ){
		free(linea);
		return -1;
	}
	else{

		/*comprobar si ya existe username*/
		while(getline(&linea,&len,fp) != -1){
			/*leer linea a linea hasta encontrar usuario*/
			pch = strtok(linea," ");
			if(strcmp(pch,(*j)->username) == 0){
				free(linea);
				return 0;
			}
		}

		/*si no existe, guardar al final*/
		if(strcmp((*j)->username,"DESCONOCIDO")!=0 && strlen((*j)->username)>0){
			fprintf(fp, "%s %s\n", (*j)->username, (*j)->pass);
			fclose(fp);
			free(linea);
			return 1;
		}
		else{
			free(linea);
			return -1;
		}
	}
}

/**
 * Logea un jugador. Busca en un fichero un jugador, comprueba credenciales
 * y lo marca como logeado. Si hay error devuelve -1. Si credenciales incorrectos
 * devuelve -1.
 */
int jugador_login(Jugador** j){
	FILE *fp;
	char *linea = NULL;
	char * pch;
	size_t len = 0;
	size_t read;

	fp = fopen("users.txt","r");
	if(fp == NULL){
		return -1;
	}
	else{
		/*comprobar si existe username*/
		while(getline(linea,250,fp) != -1){
			/*leer linea a linea hasta encontrar usuario*/
			pch = strtok(linea," ");
			if(strcmp(pch,(*fp)->username) == 0){
				if(strcmp(strtok(NULL," "),(*fp)->pass) == 0){
					(*j)->logeado = 1;
					fclose(fp);
					return 1;
				}
				else{
					fclose(fp);
					return 0;
				}
			}
		}
		fclose(fp);
		return 0;
	}
}