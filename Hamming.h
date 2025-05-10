#ifndef Hamming_h
#define Hamming_h

#include <stdint.h>

#define TAM_CADENAS_NOMBRE 128
#define SIZEOF_UINT32 4
#define SIZEOF_UINT16 2
#define SIZEOF_UINT8 1

//bloques de 8
#define NUM_BITS_TOTAL_8 8
#define TAM_ARREGLO_8 1
#define NUM_BITS_INFO_8 4
#define NUM_BITS_CONTROL_8 3

//bloques de 256
#define NUM_BITS_TOTAL_256 256
#define TAM_ARREGLO_256 32
#define NUM_BITS_INFO_256 247
#define NUM_BITS_CONTROL_256 8

//bloques de 4096
#define NUM_BITS_TOTAL_4096 4096
#define TAM_ARREGLO_4096 512
#define NUM_BITS_INFO_4096 4083
#define NUM_BITS_CONTROL_4096 12


enum hamming_tam_bloque{ HAM8, HAM256, HAM4096 };

enum hamming_estados_bloque{ EST_SINERROR, EST_UN_ERROR, EST_DOS_ERRORES };
// constantes para facilitar la deteccion automatica de tipo de codificacion
enum extension { NONE = -1, TXT, HA1, HA2, HA3, HE1, HE2, HE3, DE1, DE2, DE3, DC1, DC2, DC3 };

// buffer bits
struct palabra_buffer {
	uint8_t p;
	short int bits_restantes;
};

struct sindrome {
	uint16_t bits_control;
	uint8_t paridad_total;
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

// operaciones con codigos de Hamming
int _hamming_codificar_bloque(
		const int,
		struct buffer_bits *,
		struct buffer_bits *,
		int);
int _hamming_codificar_archivo(const int, char []);
struct sindrome _hamming_decodificar_bloque(
		const int,
		struct buffer_bits *,
		struct buffer_bits *);
int _hamming_decodificar_archivo(const int, char []);
int _hamming_error_en_bloque(const int, struct buffer_bits *, float);
int _hamming_error_en_archivo(const int, char [], float);
void _hamming_corregir_info(const int, struct buffer_bits *, struct sindrome );

// operaciones solo para bloques de 8 bits
uint16_t _hamming_codificar_bloque_8(uint8_t );
int _hamming_codificar_archivo_8(char []);
uint16_t _hamming_decodificar_bloque_8(uint16_t , uint8_t *);
int _hamming_decodificar_archivo_8(char []);
int _hamming_error_en_bloque_8(uint8_t *,float);
int _hamming_error_en_archivo_8(char [], float);
void _hamming_corregir_bloque_8(uint16_t *, uint16_t , int []);

// funciones adicionales
void nombre_archivo_quitar_extension(char [], char []);
uint16_t indice_restar_bits_control(uint16_t );
int tipo_ext_nombre_archivo(char []);

#endif  
