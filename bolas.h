#ifndef BOLAS_H
#define BOLAS_H

#include <stdio.h>

typedef
struct bola{
	int num;
	struct bola* next;
} Bola;

void bombo_gen(Bola **bombo, int len, int desde);
int bombo_size(Bola *bombo);
int bombo_pop(Bola **bombo, int* len);
int bombo_clean(Bola **bombo);
void print_bombo(Bola *bombo);
int bombo_find(Bola *bombo, int num);

#endif
