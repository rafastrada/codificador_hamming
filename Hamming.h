#ifndef Hamming_h
#define Hamming_h

#include <stdint.h>

/* #define TAM_ARREGLO_4096 128 */
/*DEBUG CAMBIAR DESPUES!! */
#define TAM_ARREGLO_4096 2
/* #define NUM_BITS_INFO_4096 4083 */
/*DEBUG CAMBIAR DESPUES */
#define NUM_BITS_INFO_4096 57
/* #define NUM_BITS_TOTAL_4096 4096 */
/*DEBUG CAMBIAR DESPUES */
#define NUM_BITS_TOTAL_4096 64

#define TAM_CADENAS_NOMBRE 128
#define SIZEOF_UINT32 4
#define SIZEOF_UINT16 2
#define SIZEOF_UINT8 1


enum hamming_tam_bloque{ HAM8, HAM4096, HAM65536 };

enum hamming_estados_bloque{ EST_SINERROR, EST_UN_ERROR, EST_DOS_ERRORES };


uint16_t _hamming_codificar_bloque_8(uint8_t );
/* void _hamming_codificar_bloque_4096(uint32_t [], uint32_t []); */
uint8_t _hamming_codificar_bloque_4096(uint32_t [],uint8_t , uint16_t , uint32_t []);

int _hamming_codificar_archivo_8bits(char []);

int _hamming_codificar_archivo_4096bits(char []);

uint16_t _hamming_decodificar_bloque_8(uint16_t , uint8_t *);

uint16_t _hamming_decodificar_bloque_4096(uint32_t [], uint8_t ,uint32_t []);

int _hamming_decodificar_archivo_8bits(char []);

void _hamming_corregir_bloque_8(uint8_t *, uint16_t , int []);


#endif  // INCLUDE/home/gestrada/Universidad/2024_1c/Teoria de la informacion y la comunicacion/trabajo_maquina_1HammingHamming.h_
