#ifndef CARTONES_H
#define CARTONES_H

#include <stdio.h>
#include "bolas.h"

typedef
struct carton{
  int numeros[15];
}Carton;

void carton_nuevo(int carton[3][9]);
void carton_print(int carton[3][9]);
void carton_str(char** buffer, int carton[3][9]);

#endif
