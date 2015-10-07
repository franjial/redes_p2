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


	return 1;
}
int jugador_logout(Jugador* jug){}
int jugador_bingo(const Jugador* jug){}
int jugador_linea(const Jugador* jug){}
int jugador_slinea(const Jugador* jug){}

/**
 * Registra un usuario. Si existe alguien con el username indicado
 * devuelve 0. Si se registra bien, devuelve 1.
 * Si error devuelve un codigo menor que 0.
 */
int jugador_registrar(const char* username, const char* pass){
	FILE *fp;
	char* linea = NULL;
	char* pch;
	int len = 170;
	linea = (char *)malloc(len+1);

	if( strlen(username)==0 || strcmp(username,"DESCONOCIDO")==0 ){
		return -1;
	}


	if( (fp=fopen("users.txt","r+")) == NULL ){
		free(linea);
		return -1;
	}
	else{

		/*comprobar si ya existe username*/
		while(getline(&linea,&len,fp) != -1){
			/*leer linea a linea hasta encontrar usuario*/
			pch = strtok(linea," ");
			if(strcmp(pch,username) == 0){
				free(linea);
				return 0;
			}
		}


		fprintf(fp, "%s %s\n", username, pass);
		fclose(fp);
		free(linea);
		return 1;

	}
}

/**
 * Logea un jugador. Busca en un fichero un jugador, comprueba credenciales
 * y lo marca como logeado. Si hay error devuelve -1. Si credenciales incorrectos
 * devuelve -1.
 */
int jugador_login(Jugador** j, const char pass[128]){
	FILE *fp;
	char* linea = NULL;
	char* pch;
	int len = 170;




	fp = fopen("users.txt","r");
	if(fp == NULL){
		return -1;
	}
	else{
		/*comprobar si existe username*/
		while(getline(&linea,&len,fp) != -1){
			/*leer linea a linea hasta encontrar usuario*/
			pch = strtok(linea," ");
			if(strcmp(pch,(*j)->username) == 0){
				/*comprobar password*/
				pch = strtok(NULL," ");
				if(strcmp(pch,pass) == 0){
					(*j)->logeado = 1;
					fclose(fp);
					return 1;
				}
				else{
					(*j)->logeado = 0;
					fclose(fp);
					return 0;
				}
			}
			else{
				/*usuario no registrado*/
				(*j)->logeado=0;
			}
		}
		fclose(fp);
		return 0;
	}
}


/**
 * Devuelve 1 si el jugador se encuentra registrado, y 0 en caso contrario.
 * -1 si error de lectura de fichero
 */
int jugador_registrado(const char username[40]){
	FILE *fp;
	char* linea = NULL;
	char* pch;
	int len = 170;


	fp = fopen("users.txt","r");
	if(fp == NULL){
		return -1;
	}
	else{
		/*comprobar si existe username*/
		while(getline(&linea,&len,fp) != -1){
			/*leer linea a linea hasta encontrar usuario*/
			pch = strtok(linea," ");
			if(strcmp(pch,username) == 0){
				/*encontrado*/
				fclose(fp);
				return 1;
			}

		}

		/*no registrado*/
		fclose(fp);
		return 0;
	}
}