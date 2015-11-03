#include <stdio.h>
#include "cartones.h"
#include "bolas.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>


void carton_nuevo(int carton[3][9]){
  int i,j;
  Bola* bombo = NULL;
  int vacias = 27; /*27 posiciones a rellenar*/
  int n=-1;
  int len = 90;

  /*inicializar carton con posiciones vacias*/
  for(i=0;i<3;i++){
    for(j=0;j<9;j++){
      carton[i][j]=0;
    }
  }


  bombo_gen(&bombo,len,1); /*90 bolas empezando por la 1*/
  while(vacias!=0 || bombo!=NULL){
    n=bombo_pop(&bombo);

    if(n<10){
      for(i=0;i<3;i++){
        if(carton[i][0]==0){
          carton[i][0]=n;
          vacias--;
          break;
        }
      }
    }
    else if(n<20){
      for(i=0;i<3;i++){
        if(carton[i][1]==0){
          carton[i][1]=n;
          vacias--;
          break;
        }
      }
    }
    else if(n<30){
      for(i=0;i<3;i++){
        if(carton[i][2]==0){
          carton[i][2]=n;
          vacias--;
          break;
        }
      }
    }
    else if(n<40){
      for(i=0;i<3;i++){
        if(carton[i][3]==0){
          carton[i][3]=n;
          vacias--;
          break;
        }
      }
    }
    else if(n<50){
      for(i=0;i<3;i++){
        if(carton[i][4]==0){
          carton[i][4]=n;
          vacias--;
          break;
        }
      }
    }
    else if(n<60){
      for(i=0;i<3;i++){
        if(carton[i][5]==0){
          carton[i][5]=n;
          vacias--;
          break;
        }
      }
    }
    else if(n<70){
      for(i=0;i<3;i++){
        if(carton[i][6]==0){
          carton[i][6]=n;
          vacias--;
          break;
        }
      }
    }
    else if(n<80){
      for(i=0;i<3;i++){
        if(carton[i][7]==0){
          carton[i][7]=n;
          vacias--;
          break;
        }
      }
    }
    else{
      for(i=0;i<3;i++){
        if(carton[i][8]==0){
          carton[i][8]=n;
          vacias--;
          break;
        }
      }
    }

  }
  bombo_clean(&bombo);



  srand(time(NULL));
  for(i=0;i<3;i++){
    n=4;
    while(n>0){
      j=rand()%9;
      if(carton[i][j]!=-1){
        carton[i][j]=-1;
        n--;
      }
    }
  }
}

void carton_print(int carton[3][9]){
  int i,j;
  for(i=0;i<3;i++){
    for(j=0;j<9;j++){
      printf("%d\t",carton[i][j]);
    }
    printf("\n");
  }
}

/**
 * carton_bingo(int carton[3][9], Bola* bolas_fuera)
 *
 * indica si el carton tiene bingo
 *
 * argumento 1: carton del jugador matriz 3x9
 * argumento 2: lista de bolas que han salido del bombo
 *
 * devuelve 1 si el carton tiene bingo
 *          0 si el carton no tiente bingo
 *
 */
int carton_bingo(int carton[3][9], int bolas_fuera[90]){
	int i,j;

	/**
	 * si encuentro numero en carton que no esta en bolas_fuera
	 * devuelvo 0
	 */

	for(i=0;i<3;i++){
		for(j=0;j<9;j++){
			if(carton[i][j] > 0 && carton[i][j]<=90 && bolas_fuera[carton[i][j]-1] == 0){
				printf("Falta %d\n",carton[i][j]);
				return 0;
			}
		}
	}


	return 1;
}


int carton_linea(int carton[3][9], int bolas_fuera[90]){
  int i,j;
  int test;
  for(i=0;i<3;i++){
    test = 1;
    for(j=0;j<9;j++){
      if(carton[i][j] > 0 && carton[i][j]<=90 && bolas_fuera[carton[i][j]-1]==0){
        test = 0;
      }
    }
    if(test==1){
      return 1;
    }
  }

  return 0;
}


int carton_slinea(int carton[3][9], int bolas_fuera[90]){
  int i,j;
  int test;
  int count=0;

  for(i=0;i<3;i++){
    test = 1;
    for(j=0;j<9;j++){
      if(carton[i][j] > 0 && carton[i][j]<=90 && bolas_fuera[carton[i][j]-1]==0){
        test = 0;
      }
    }
    if(test==1){
      count++;
    }
  }

  if(count==2)
    return 1;
  else
    return 0;
}



void carton_str(char buffer[250], int carton[3][9]){
  int i,j;
  char aux[4];

  strcpy(buffer,"CARTON|");
  for(i=0;i<3;i++){
    for(j=0;j<9;j++){
      if(carton[i][j]==-1)
        sprintf(buffer,"%sX, ",buffer,carton[i][j]);
      else
        sprintf(buffer,"%sNum.%d, ",buffer,carton[i][j]);
    }
  }

}
