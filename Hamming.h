#ifndef Hamming_h
#define Hamming_h

#include <stdint.h>
#include <stdio.h>

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

//bloques de 256
#define NUM_BITS_TOTAL_256 256
#define TAM_ARREGLO_256 32
#define NUM_BITS_INFO_256 247
#define NUM_BITS_CONTROL_256 8

enum hamming_tam_bloque{ HAM8, HAM256, HAM4096 };

enum hamming_estados_bloque{ EST_SINERROR, EST_UN_ERROR, EST_DOS_ERRORES };

// buffer bits
struct palabra_buffer {
	uint8_t p;
	short int bits_restantes;
};

struct buffer_intervalo {
	uint8_t *inicio, *fin;
	short int inicio_bits_disponibles;
};

struct buffer_bits {
	uint8_t *arreglo;		// puntero a base de arreglo de palabras
	const unsigned int tam_arreglo;	// tamanio de arreglo de palabras
	uint8_t *palabra_inicio, *palabra_ultima;
	short int palabra_inicio_bits;	// bits disponibles en la palabra de inicio
	short int palabra_ultima_bits;
};
void buffer_bits_init(struct buffer_bits *buffer);
void buffer_bits_free(struct buffer_bits *buffer);

// para bloques de 256
int _hamming_codificar_bloque_256(struct buffer_bits *, struct buffer_bits *, int);
int _hamming_codificar_archivo_256(char []);

uint16_t _hamming_codificar_bloque_8(uint8_t );

uint8_t _hamming_codificar_bloque_4096(uint32_t [],uint8_t , uint16_t , uint32_t []);

int _hamming_codificar_archivo_8bits(char []);

int _hamming_codificar_archivo_4096bits(char []);

uint16_t _hamming_decodificar_bloque_8(uint16_t , uint8_t *);

uint16_t _hamming_decodificar_bloque_4096(uint32_t [], uint8_t ,uint32_t []);

int _hamming_decodificar_archivo_4096bits(char []);

int _hamming_decodificar_archivo_8bits(char []);

int _hamming_error_en_bloque_8(uint8_t *,float);

int _hamming_error_en_archivo_8bits(char [], float);

void _hamming_corregir_bloque_8(uint16_t *, uint16_t , int []);

// funciones adicionales
void nombre_archivo_quitar_extension(char [], char []);

#endif  
