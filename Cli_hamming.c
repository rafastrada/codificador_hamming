#include "Hamming.h"
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// constantes para facilitar la deteccion automatica de tipo de codificacion
enum extension { NAN = -1, TXT, HA1, HA2, HA3, HE1, HE2, HE3 };
const char* ext_strings[] = {
	".txt", ".ha1",".ha2",".ha3",".he1",".he2",".he3"
};

// prototipos de funciones
int tipo_ext_nombre_archivo(char []);


/*
 * El programa recibe por parametro que funcion va a realizar:
 * codificar o decodificar un archivo.
 * Se indica por medio del primer parametro 'codificar' o 'decodificar',
 * en el primer caso se indican tambien el tamanio del bloque (8,256,4096)
 * y el nombre del archivo (con extension) como parametros adicionales.
 * */
int main(int argc, char *argv[])
{
	// mensaje inicial
	printf("Codificador de archivos TXT con Codigos de Hamming\n\n");

	/* se analizan los parametros de entrada */
	/* si no se incluyeron parametros (menos de 2) solo imprime
	 * la ayuda */
	if (argc < 2) {
		printf("Formato de argumentos del programa:\n"
				"cli_hamming.exe OPERACION ...\n\n"
				"Operaciones disponibles:\n"
				"\tcodificar (8|256|4096) (nombre_archivo)\n"
				"\t\"Codifica el archivo con codigos de Hamming en el "
				"tamanio de bloque indicado\"\n\n"
				"\talterar (nombre_archivo) (probabilidad)\n"
				"\t\"Introduce errores en el archivo indicado segun 'probabilidad',"
				" (valor entre 0 y 1)\"\n\n"
				);

	}
	// caso que se hayan incluido parametros
	else {
		// -----------------------------------------------------------------
		// funcion CODIFICAR
		if (strcmp(argv[1], "codificar") == 0 &&
				argc == 4) {	//controla cantidad de parametros

			char nombre_archivo_entrada[TAM_CADENAS_NOMBRE], 
				nombre_archivo_salida[TAM_CADENAS_NOMBRE];
			FILE *fuente, *destino;			// archivos de entrada y salida.
			int bytes_informacion = 0;		// tamanio de archivo de informacion de entrada,
											// usado para imprimir en pantalla.

			// copia el 4to parametro, que deberia ser el nombre del archivo
			strncpy(nombre_archivo_entrada, argv[3], TAM_CADENAS_NOMBRE);		
			// se controla que sea un archivo TXT
			if (tipo_ext_nombre_archivo(nombre_archivo_entrada) != TXT) {
				// se informa y se termina el programa como error
				printf("\nSolo se admiten por entrada archivos con extension TXT!\n");
				return EXIT_FAILURE;
			}

			// se comprueba y abre el archivo fuente
			fuente = fopen(nombre_archivo_entrada, "rb");
			if (fuente != NULL) {

				/*controla el parametro de tamanio de bloque*/
				// bloques de 8bits
				if (strcmp(argv[2], "8") == 0) {
					// crear nombre de archivo de salida
					// <nombre_archivo>.ha1
					nombre_archivo_quitar_extension(
							nombre_archivo_salida, 
							nombre_archivo_entrada);
					strcat(nombre_archivo_salida, ".ha1");	// agrega extension
															//
					// abrir archivo destino
					destino = fopen(nombre_archivo_salida, "wb+");

					//codificacion del archivo de entrada
					bytes_informacion = _hamming_codificar_archivo_8bits(fuente, destino);
					
					fclose(destino);

					printf(
							"\nSe codifico el archivo '%s' con bloques de 8 bits.\n"
							"Nombre de archivo de salida: %s\n"
							"Tamanio de archivo de salida: %d\n",
							nombre_archivo_entrada,
							nombre_archivo_salida,
							bytes_informacion
						  );
				}


				fclose(fuente);
			} else {	// caso de error al abrir el archivo
				// imprime por pantalla y termina el programa
				printf("\nHubo un error al intentar abrir el archivo %s.\n",
						nombre_archivo_entrada);
				return EXIT_FAILURE;
			}
		}

		// -------------------------------------------------------
		// funcion ALTERAR
		else if (strcmp(argv[1], "alterar") == 0 && argc == 4) {
			printf("\n\n%d\n\n",tipo_ext_nombre_archivo(argv[2]));
		}
	}


	return EXIT_SUCCESS;
}

/** Devuelve el tipo de extension que contiene el nombre de archivo indicado.
 *
 * @param nombre_archivo Nombre de archivo (solo ha1,ha2,ha3,he1,he2,he3).
 * @return Enumerado de tipo 'extension'. NAN si no es una extension admitida.
 */
int tipo_ext_nombre_archivo(char nombre_archivo[]) {
	char *ext_char = strrchr(nombre_archivo, '.');	// puntero a solamente la extension

	for (int i = 0; i<=HE3; i++) {
		if (strcmp(ext_char, ext_strings[i]) == 0)
			return i;
	}

	return NAN;
}
