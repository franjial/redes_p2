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

  for(i=desde+1;i < len;i++){
    cursor->next = (Bola *) malloc(sizeof(Bola));
    cursor = cursor->next;
    cursor->next = NULL;
    cursor->num = i;
  }

  /*ultima bola, que apunta a la primera*/
  cursor->next = (Bola*) malloc(sizeof(Bola));
  cursor = cursor->next;
  cursor->next = *bombo;
  cursor->num = i++;

}

/**
 * Genera una lista circular de 90 bolas del 1 al 90
 * recibe una lista vacia de elementos.
 */
void generar(Bola **bombo,int* len){
	int i;
	Bola *cursor;

	/*primer elemento*/
	*bombo = (Bola *) malloc(sizeof(Bola));
	(*bombo)->next=NULL;
	(*bombo)->num=1;
	cursor = *bombo;

	for (i=2; i<=89; i++){
		cursor->next = (Bola *) malloc(sizeof(Bola));
		cursor = cursor->next;
		cursor->next = NULL;
		cursor->num = i;
	}

	/*ultimo elemento*/
	cursor->next = (Bola *) malloc(sizeof(Bola));
	cursor = cursor->next;
	cursor->next = *bombo;
	cursor->num = 90;

	*len = 90;
}

/**
 * Devuelve numero de bolas que quedan en el bombo
 */
int bombo_size(Bola *bombo){
  Bola* cursor = bombo;
  int i = 1;

  /*sale si vacio o llega a ultima bola*/
  while(cursor->next != bombo && cursor != NULL){
    cursor = cursor->next;
    i++;
  }
  return i;
}


/**
 * Saca del bombo una bola al azar
 */
int bombo_pop(Bola **bombo,int *len){
	int pos;
	int i;
	int num;
	Bola* cursor = *bombo; /*cursor al principio inicialmente*/
	Bola* padre = NULL;
	Bola* sucesor = NULL;


	/*si el bombo esta vacio*/
	if(*bombo==NULL){
		return -1;
	}


	/*si solo hay una bola*/
	if(*bombo == (*bombo)->next){
	  num = (*bombo)->num;
	  free(*bombo);
	  *bombo = NULL;
	  if(len!=NULL){
	  	*len = 0;
	  }
	  return num;
	}

	/*si hay mas de una bola en el bombo*/

	srand(time(NULL));

	if(len==NULL){
		pos = rand()%bombo_size(*bombo);
	}else{
		pos = rand()%(*len); /*numero pseudoaleatorio entre 0-(len-1)*/
	}

	/*dejo el cursor en el padre de la posicion seleccionada*/
	for(i=0;i<pos-1;i++){
	  cursor = cursor->next;
	}

	padre = cursor;
	cursor = cursor->next;
	sucesor = cursor->next;
	num = cursor->num;

	/**
	 * sacar bola del bombo y liberar memoria
	 */

	padre->next = sucesor;
	free(cursor);
	if(len!=NULL)
		*len=*len - 1;

	return num;
}


/**
 * Vacia el bombo. Devuelve bolas sacadas
 */
int bombo_clean(Bola **bombo){

  if(*bombo==NULL){
  	return 0;
  }

  int i=0;
  Bola *head = *bombo; /*PRIMERA BOLA*/
  Bola *cursor = *bombo;

  if(head == head->next){ /*bombo con una bola*/
    free(head);
    return 1;
  }

  *bombo = (*bombo)->next;
  cursor = (*bombo);
  while(head != head->next->next){ /*mientras queden mas de dos bolas*/
    cursor = cursor->next;
    free(*bombo);
    head->next=cursor;
    *bombo=cursor;
    i++;
  }

  /*quedan dos bolas*/
  free( head->next );
  free( head );
  *bombo = NULL;

  /*no debe llegar aqui nunca*/
  return i+2;
}

/**
 *
 */
void print_bombo(Bola *bombo){
	Bola *cursor = bombo;

	if(bombo==NULL){
		printf("Bombo vacio\n");
	}
	else{
		while(bombo != cursor->next){
			printf("%d, ",cursor->num);
			cursor=cursor->next;
		}
		printf("%d\n",cursor->num);
	}
}

/**
 * devuelve 1 si num esta en el bombo, 0 en caso
 * contrario.
 */
int bombo_find(Bola *bombo, int num){
	Bola *cursor = bombo;

	if(bombo==NULL){
		/*bombo vacio*/
		return 0;
	}
	else{
		while(bombo != cursor->next){
			/*recorro toda la lista*/
			if(cursor->num != num)
				return 0;
			cursor=cursor->next;
		}
		/*test de la ultima posicion*/
		if(cursor->num == num)
			return 1;
		else
			return 0;
	}
}