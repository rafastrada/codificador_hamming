#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "Hamming.h"

#define ROJO "\x1b[31m"
#define RESET "\x1b[0m"

int main(int argc, char *argv[])
{
	printf("Salida: %" PRIx16 "\n", _hamming_codificar_bloque_8(0x0f));
	printf("Salida: %" PRIx16 "\n", _hamming_codificar_bloque_8(0xf0));
	printf("Salida: %" PRIx16 "\n", _hamming_codificar_bloque_8(0xa0));
	printf("Salida: %" PRIx16 "\n", _hamming_codificar_bloque_8(0x60));
	printf("Salida: %" PRIx16 "\n", _hamming_codificar_bloque_8(0x6f));
	printf("Salida: %" PRIx16 "\n", _hamming_codificar_bloque_8(0x6a));

	/*prueba de funcion de 4096 */
	uint32_t informacion[] = {UINT32_MAX,UINT32_MAX}, codificado[2], decodificado[2],
			informacion2[] = {0x12348765, 0xff00ccaa};

	uint8_t bits_sobrantes = _hamming_codificar_bloque_4096(informacion2,32,57, codificado);

	printf("Entrada: %" PRIx32 " %" PRIx32 "\n",informacion2[0],informacion2[1]);
	printf("Salida: %" PRIx32 " %" PRIx32 "\n",codificado[0],codificado[1]);
	printf("Bits sobrantes: %" PRIx8 "\n", bits_sobrantes);

	/* int salida = _hamming_codificar_archivo_8bits("prueba"); */

	int salida = _hamming_decodificar_archivo_8bits("prueba");

	uint8_t info; uint16_t bloque = 0x33d1, sindrome;

	sindrome = _hamming_decodificar_bloque_8(bloque, &info);

	printf("Bloque: %" PRIx16 ROJO "\tInfo: %" PRIx8 RESET "\tSindrome: %" PRIx16 "\n", bloque,info,sindrome);

	/* codificado[1] = 0x7fc019c4; */
	codificado[1] = 0x7fc01945;
	uint16_t sindrome2 = _hamming_decodificar_bloque_4096(codificado, 32, decodificado);
	printf("Codificado: %" PRIx32 " %" PRIx32 "\n",codificado[0],codificado[1]);
	printf("Decodificado: %" PRIx32 " %" PRIx32 "\n",decodificado[0],decodificado[1]);
	printf("Sindrome: %" PRIx16 "\n", sindrome2);


	return salida;
}
