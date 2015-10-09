#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jugador.h"
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

static Jugador *jugador = NULL;

int init_suite(void){
   return 0;
}

int clean_suite(void){

  return 0;
}

void test_crear(){

}

void test_registrar(void){

}

void test_login(void){

}

void test_clean(void){

}

void test_logout(void){
	
}



int
main(){
  CU_pSuite pSuite = NULL;

  if(CUE_SUCCESS != CU_initialize_registry())
    return CU_get_error();

  pSuite = CU_add_suite("Test jugadores", init_suite, clean_suite);
  if(NULL == pSuite){
    CU_cleanup_registry();
    return CU_get_error();
  }

  /*agregar tests a la suite*/
  if( (NULL == CU_add_test(pSuite, "crear jugador", test_crear))||
      (NULL == CU_add_test(pSuite, "registrar jugador", test_registrar))||
      (NULL == CU_add_test(pSuite, "login jugador", test_login))||
      (NULL == CU_add_test(pSuite, "logout jugador", test_logout))||
      (NULL == CU_add_test(pSuite, "eliminar jugador", test_clean)) ){
    CU_cleanup_registry();
    return CU_get_error();
  }

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();

  return CU_get_error();
}
