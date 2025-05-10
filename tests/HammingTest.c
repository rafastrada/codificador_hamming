#include <CUnit/Basic.h>
#include <CUnit/CUError.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>
#include <CUnit/TestRun.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "Hamming.h"



void test_hamming_bloque_8_bits();

int main(int argc, char *argv[])
{
	if (CUE_SUCCESS != CU_initialize_registry()) {
		return CU_get_error();
	}

	CU_basic_set_mode(CU_BRM_VERBOSE);
	
	CU_pSuite suite = NULL;


	suite = CU_add_suite("testHamming", 0,0);
	if (suite == NULL) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	if (NULL == CU_add_test(suite, "test_hamming_8_bits", test_hamming_bloque_8_bits)) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	CU_basic_run_tests();

	return CU_get_error();
}


void test_hamming_bloque_8_bits() {
	uint8_t entrada = 0x0;
	uint16_t salida_esperada = 0x0;

	CU_ASSERT_EQUAL(_hamming_codificar_bloque_8(entrada), salida_esperada);
	CU_ASSERT_EQUAL(_hamming_codificar_bloque_8(0x0f), 0x00ff);
}
