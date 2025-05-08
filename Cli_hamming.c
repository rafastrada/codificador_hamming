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

	char nombre_archivo_entrada[TAM_CADENAS_NOMBRE];

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
				bloques_escritos = _hamming_codificar_archivo_8bits(nombre_archivo_entrada);
				
				printf(
						"\nSe codifico el archivo '%s' con bloques de 8 bits.\n"
						"Se creo un archivo de formato HA1 con el mismo nombre.\n"
						"Cantidad de bloques en el archivo de salida: %d\n",
						nombre_archivo_entrada,
						bloques_escritos
					  );
			}
			// bloques de 256bits
			else if (strcmp(argv[2],"256") == 0) {
				bloques_escritos =
					_hamming_codificar_archivo_256(nombre_archivo_entrada);


				printf(
						"\nSe codifico el archivo '%s' con bloques de 256 bits.\n"
						"Se creo un archivo de formato HA2 con el mismo nombre.\n"
						"Cantidad de bloques en el archivo de salida: %d\n",
						nombre_archivo_entrada,
						bloques_escritos
					  );
			}
			// bloques de 4096bits
			else if (strcmp(argv[2],"4096") == 0) {
				// codificacion del archivo de entrada
				bloques_escritos =
					_hamming_codificar_archivo_4096bits(nombre_archivo_entrada);
				
				printf(
						"\nSe codifico el archivo '%s' con bloques de 4096 bits.\n"
						"Se creo un archivo de formato HA3 con el mismo nombre.\n"
						"Cantidad de bloques en el archivo de salida: %d\n",
						nombre_archivo_entrada,
						bloques_escritos
					  );
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
					_hamming_error_en_archivo_8bits(
							nombre_archivo_entrada, probabilidad);
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
			
			switch (extension_nombre_archivo) {
				case HA1:
				case HE1:
					// decodificacion de bloques de 8 bits
					_hamming_decodificar_archivo_8bits(nombre_archivo_entrada);
					break;
				case HA3:
				case HE3:
					// decodificacion de bloques de 4096
					_hamming_decodificar_archivo_4096bits(nombre_archivo_entrada);
					break;
				
				/*si el archivo no tiene una extension valida*/
				default:
					printf("\nDebe ingresar un archivo de formato .ha_ o .he_ !\n");
					return EXIT_FAILURE;
			}
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
