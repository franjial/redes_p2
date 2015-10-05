#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bolas.h"
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

static Bola *bombo = NULL;
static int nbolas = 90;

int init_suite(void){
   return 0;
}

int clean_suite(void){
  if(bombo==NULL){
    return 0;
  }
  
  Bola* cursor = bombo;
  int i;
  
  for(i=0;i<nbolas;i++){
    cursor = bombo->next;
    free(bombo);
    bombo = cursor;
  }
  
  return 0;
}

void test_generar(void){
  int i;
  Bola* cursor;
  
  bombo_gen(&bombo,nbolas,1); /*bombo con 90 bolas de 1 a 90*/
  cursor = bombo; /*cursor inicialmente en head*/
  
  for(i=1;i<=nbolas;i++){
    CU_ASSERT(cursor->num == i);
    cursor = cursor->next;
  }
}

void test_len(void){
  bombo_gen(&bombo,90,1);
  CU_ASSERT(90==bombo_size(bombo));
}

void test_clean(void){
  bombo_gen(&bombo,90,1);
  CU_ASSERT(90==bombo_clean(&bombo));
}

void test_sacar(void){
  int i;
  int sacada;
  int l;
  Bola* cursor;
  int encontrado = 0;
  nbolas=10;

  bombo_gen(&bombo,nbolas,1);
  cursor = bombo;
  
  sacada = bombo_pop(&bombo,&nbolas);

  CU_ASSERT(nbolas==9);
  CU_ASSERT(nbolas==bombo_size(bombo));

  /*la bola sacada no debe estar dentro del bombo*/
  for(i=0;i<nbolas;i++){
    CU_ASSERT_FALSE(cursor->num == sacada);
    cursor = cursor->next;
  }

}


int
main(){
  CU_pSuite pSuite = NULL;

  if(CUE_SUCCESS != CU_initialize_registry())
    return CU_get_error();

  pSuite = CU_add_suite("Suite", init_suite, clean_suite);
  if(NULL == pSuite){
    CU_cleanup_registry();
    return CU_get_error();
  }

  /*agregar tests a la suite*/
  if( (NULL == CU_add_test(pSuite, "generando bolas", test_generar))||
      (NULL == CU_add_test(pSuite, "sacando una bola", test_sacar))||
      (NULL == CU_add_test(pSuite, "contando bolas", test_len))||
      (NULL == CU_add_test(pSuite, "limpiando el bombo", test_clean)) ){
    CU_cleanup_registry();
    return CU_get_error();
  }

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();
  
  return CU_get_error();
}
