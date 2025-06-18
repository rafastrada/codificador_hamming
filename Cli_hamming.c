#include "Hamming.h"
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// prototipos
int out_mensaje_codificacion(int tipo_ham, char nombre_archivo[], int bloques_escritos);
int out_mensaje_decodificacion(int tipo_ham, char nombre_archivo[], int nivel_error);

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

	char nombre_archivo_entrada[TAM_CADENAS_NOMBRE];
	int retorno_programa = EXIT_SUCCESS;

	/* se analizan los parametros de entrada */
	/* si no se incluyeron parametros (menos de 2) solo imprime
	 * la ayuda */
	if (argc < 2) {
		printf("Formato de argumentos del programa:\n"
				"cli_hamming.exe OPERACION ...\n\n"
				"Operaciones disponibles:\n"
				//codificar
				"\tcodificar (8|256|4096) (nombre_archivo)\n"
				"\t\"Codifica el archivo con codigos de Hamming en el "
				"tamanio de bloque indicado\"\n\n"
				//alterar
				"\talterar (nombre_archivo) (probabilidad)\n"
				"\t\"Introduce errores en el archivo indicado segun 'probabilidad',"
				" (valor entre 0 y 1)\"\n\n"
				//decodificar
				"\tdecodificar (nombre_archivo)\n"
				"\t\"Decodifica el archivo indicado y escribe la informacion en dos\n"
				"\tnuevos archivos, uno sin corregir y el otro corregido\"\n\n"
				);

	}
	// caso que se hayan incluido parametros
	else {
		// -----------------------------------------------------------------
		// funcion CODIFICAR
		if (strcmp(argv[1], "codificar") == 0 &&
				argc == 4) {	//controla cantidad de parametros

			int bloques_escritos = 0;		// cantidad de bloques escritos,
											// usado para imprimir en pantalla.

			// copia el 4to parametro, que deberia ser el nombre del archivo
			strncpy(nombre_archivo_entrada, argv[3], TAM_CADENAS_NOMBRE);		
			// se controla que sea un archivo TXT
			if (tipo_ext_nombre_archivo(nombre_archivo_entrada) != TXT) {
				// se informa y se termina el programa como error
				printf("\nSolo se admiten por entrada archivos con extension TXT!\n");
				return EXIT_FAILURE;
			}

			/*controla el parametro de tamanio de bloque*/
			// bloques de 8bits
			if (strcmp(argv[2], "8") == 0) {
														//
				//codificacion del archivo de entrada
				bloques_escritos = _hamming_codificar_archivo_8(nombre_archivo_entrada);

				// imprime por pantalla el mensaje correspondiente
				retorno_programa =
					out_mensaje_codificacion(HAM8, nombre_archivo_entrada, bloques_escritos);
			}
			// bloques de 256bits
			else if (strcmp(argv[2],"256") == 0) {
				bloques_escritos =
					_hamming_codificar_archivo(HAM256, nombre_archivo_entrada);

				// imprime por pantalla el mensaje correspondiente
				retorno_programa =
					out_mensaje_codificacion(HAM256, nombre_archivo_entrada, bloques_escritos);
			}
			// bloques de 4096bits
			else if (strcmp(argv[2],"4096") == 0) {
				// codificacion del archivo de entrada
				bloques_escritos =
					_hamming_codificar_archivo(HAM4096, nombre_archivo_entrada);
				
				// imprime por pantalla el mensaje correspondiente
				retorno_programa =
					out_mensaje_codificacion(HAM4096, nombre_archivo_entrada, bloques_escritos);
			}
		}
		// -------------------------------------------------------
		// funcion ALTERAR
		// cli_hamming.exe | 'alterar' | nom_archivo | prob_de_error
		else if (strcmp(argv[1], "alterar") == 0 && argc == 4) {
			// captura de nombre de archivo de entrada (3er argumento)
			strncpy(nombre_archivo_entrada, argv[2], TAM_CADENAS_NOMBRE);
			// captura de extension de archivo de entrada
			int ext_archivo_entrada =
				tipo_ext_nombre_archivo(nombre_archivo_entrada);
			// captura de argumento de probabilidad (4to argumento),
			// se convierte de string a flotante
			const float probabilidad = strtof(argv[3], NULL);

			/* segun el tipo de codificacion se bifurca en la funcion de 
			 * introduccion de error correspondiente*/
			switch (ext_archivo_entrada) {
				case HA1:
					_hamming_error_en_archivo_8(
							nombre_archivo_entrada, probabilidad);
					break;
				case HA2:
					_hamming_error_en_archivo(
							HAM256, nombre_archivo_entrada, probabilidad);
					break;
				case HA3:
					_hamming_error_en_archivo(
							HAM4096, nombre_archivo_entrada, probabilidad);
					break;
				default:
					// cuando el archivo no es de formato HA_
					// termina el programa
					printf(
							"Debe ingresar un archivo de formato .ha_ para alterarlo!\n"
						  );
					return EXIT_FAILURE;
			}
		// -------------------------------------------------------
		// funcion DECODIFICAR
		// cli_hamming.exe | 'codificar' | nom_archivo
		} else if (strcmp(argv[1], "decodificar") == 0 && argc == 3) {
			//copia en limpio de nombre 'nom_archivo'
			strncpy(nombre_archivo_entrada, argv[2], TAM_CADENAS_NOMBRE);
			
			// se captura la extension del archivo para determinar el tamanio de bloque,
			// y utilizar el metodo de decodificacion correspondiente
			int extension_nombre_archivo =
				tipo_ext_nombre_archivo(nombre_archivo_entrada);
			int tipo_ham;
			// captura el maximo nivel de error en el archivo a decodificar
			int nivel_error;
			
			switch (extension_nombre_archivo) {
				case HA1:
				case HE1:
					// decodificacion de bloques de 8 bits
					tipo_ham = HAM8;
					nivel_error =
						_hamming_decodificar_archivo_8(nombre_archivo_entrada);
					break;
				case HA2:
				case HE2:
					// decodificacion de bloques de 256
					tipo_ham = HAM256;
					nivel_error =
						_hamming_decodificar_archivo(HAM256,nombre_archivo_entrada);
					break;
				case HA3:
				case HE3:
					// decodificacion de bloques de 4096
					tipo_ham = HAM4096;
					nivel_error =
						_hamming_decodificar_archivo(HAM4096,nombre_archivo_entrada);
					break;
				/*si el archivo no tiene una extension valida*/
				default:
					printf("\nDebe ingresar un archivo de formato .ha_ o .he_ !\n");
					return EXIT_FAILURE;
			}

			// imprime por pantalla
			retorno_programa =
				out_mensaje_decodificacion(tipo_ham, nombre_archivo_entrada, nivel_error);
		}
	}

	return retorno_programa;
}

int out_mensaje_codificacion(int tipo_ham, char nombre_archivo[], int bloques_escritos){

	// caso: error en el manejo de archivos
	if (bloques_escritos == -1) {
		printf("\nHubo un error al codificar el archivo. (existe?)\n");
		return EXIT_FAILURE;
	}
	else printf(
			"\nSe codifico el archivo '%s' con bloques de %d bits.\n"
			"Se creo un archivo de formato HA%d con el mismo nombre.\n"
			"Cantidad de bloques en el archivo de salida: %d\n",
			nombre_archivo,
			NUM_BITS_TOTAL[tipo_ham], tipo_ham+1,
			bloques_escritos
		  );
	return EXIT_SUCCESS;
}

int out_mensaje_decodificacion(int tipo_ham, char nombre_archivo[], int nivel_error) {
	int salida = 0;

	if (nivel_error == -1) {
		printf("\nHubo un error al codificar el archivo. (existe?)\n");
		return EXIT_FAILURE;
	}
	else {
		printf(
				"\nSe decodifico el archivo '%s'.\n"
				"Se crearon arhcivos de formato DE%d y DC%d con el mismo nombre.\n\n",
				nombre_archivo,
				tipo_ham+1,tipo_ham+1
			  );

		switch (nivel_error) {
			case 0:
				printf("El archivo no tenia errores.\n");
				break;
			case 1:
				printf("El archivo tenia bloques con un error."
						" Se recupero la informacion original correctamente.\n");
				break;
			case 2:
				printf("El archivo tenia bloques con mas de un error."
						"No es posible recuperar la informacion original.\n");
				salida = 2;
				break;
		}
	}

	return salida;
}
