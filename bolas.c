#include "bolas.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/*genera bolas en un rango*/
void bombo_gen(Bola **bombo, int len, int desde){
  int i=desde;
  Bola *cursor;

  /*primera bola*/
  *bombo = (Bola *) malloc(sizeof(Bola));
  (*bombo)->next = NULL;
  (*bombo)->num = i;
  cursor = *bombo;

  for(i=desde+1;i <= len;i++){
    cursor->next = (Bola *) malloc(sizeof(Bola));
    cursor = cursor->next;
    cursor->next = NULL;
    cursor->num = i;
  }

}


/**
 * Devuelve numero de bolas que quedan en el bombo
 */
int bombo_size(Bola *bombo){
  int i = 1;

  if(bombo==NULL){
	  return 0;
  }

  /*sale si vacio o llega a ultima bola*/
  while(bombo->next != NULL){
    bombo = bombo->next;
    i++;
  }
  return i;
}


/**
 * Saca del bombo una bola al azar
 */
int bombo_pop(Bola **bombo){
	int pos;
	int i;
	int num;
	int len;

	Bola* cursor = *bombo; /*cursor al principio inicialmente*/
	Bola* padre = NULL;
	Bola* sucesor = NULL;


	/*si el bombo esta vacio*/
	if(*bombo==NULL){
		return -1;
	}


	/*si solo hay una bola*/
	if( (*bombo)->next == NULL ){
	  num = (*bombo)->num;

	  free(*bombo);
	  *bombo = NULL;

	  return num;
	}

	/*si hay mas de una bola en el bombo*/

	srand(time(NULL));

	len = bombo_size(*bombo);
	pos = rand()%len; /*numero pseudoaleatorio entre 0-(len)*/

	/*caso especial pos = 0*/
	if(pos==0){
		num = cursor->num;
		*bombo = cursor->next;
		free(cursor);
	}
	else{
		/*dejo el cursor en el padre de la posicion seleccionada*/
		for(i=0;i<pos-1;i++){
		  cursor = cursor->next;
		}
		num = cursor->next->num;

		/*si hay que sacar la ultima posicion*/
		if(cursor->next->next == NULL){
			free(cursor->next);
			cursor->next = NULL;
		}
		else{
			padre = cursor;
			cursor = cursor->next;
			sucesor = cursor->next;
			num = cursor->num;
			padre->next = sucesor;
			free(cursor);
		}

	}





	return num;
}



/**
 * Vacia el bombo. Devuelve bolas sacadas
 */
int bombo_clean(Bola **bombo){
	Bola *aux;
	int i=0;

	while(*bombo != NULL){
		aux = (*bombo)->next;
		free(*bombo);
		*bombo = aux;
		i++;
	}

	return i;
}
