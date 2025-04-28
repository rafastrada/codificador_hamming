/** Modulo encargado de codificar con la tecnica de Hamming bloques de bits
 */

/* Enumeracion para determinar el tamaño de bloque con el que se trabaja
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "Hamming.h"


/** Funcion que codifica una fuente de informacion con Hamming en un arreglo nuevo.
 * La funcion puede no utilizar las 128 palabras de 32bits de entrada, por eso es que la funcion permite ingresar la cantidad de bits a usar de la primera palabra, entonces en una codificacion que sobraron bits fuente, debe ponerse al principio de un nuevo bloque fuente e indicar la cantidad de bits que quedaron fuera.
 *
 * @param bits_restantes_pInfo Cantidad de bits fuente de la primera palabra del arreglo fuente. Debe ser entre 1 y 32.
 * @param bits_restantes_total Cantidad de bits de infomrmacion. Debe ser entre 1 y 4083.
 *
 * @return Cantidad de bits sobrantes en la ultima palabra fuente.
 */
uint8_t _hamming_codificar_bloque_4096(
     uint32_t bloque_informacion[],
     uint8_t bits_restantes_pInfo,
     uint16_t bits_restantes_total,
     uint32_t bloque_codificado[]) {
	/* cuentan los bits restantes a consumir de las palabras */
	uint8_t bits_restantes_pCod;

	uint16_t bits_control = 0, /*uso de los bits:
								 b15: bit de paridad total
								 b14 - b11: sin uso
								 b10 - b0 : bits de control c10, c9, ..., c0 respectivamente
	 */
			 indice_arreglo_informacion = 0, /*va recorriendo el bloque fuente */
			 indice_arreglo_codificacion = 0; /*va recorriendo el bloque destino */
			 //bits_restantes_total = 4084;	/*bits de informacion que faltan para terminar el bloque  */
	
	uint32_t indice = NUM_BITS_TOTAL_4096,

			 /*variables temporales para leer y escribir los bloques fuente y destino */
			 palabra_codificada,
			 palabra_informacion,

			 auxiliar = 0; /* variable de usos multiples */

	/* iniciacion */
	palabra_codificada = 0x0;
	palabra_informacion = bloque_informacion[indice_arreglo_informacion] << (32 - bits_restantes_pInfo);
	/* bits_restantes_pInfo = 32; */
	bits_restantes_pCod = 32;
	

	/*hasta que el indice llegue a cero */
	/* while (indice) { */
	while (bits_restantes_total) {
		
		/*cuando se lee toda una palabra fuente, se avanza a la siguiente
		 * en el bloque de informacion, actualizando el indice y bits
		 * restantes a leer en la palabra
		 */
		if (!bits_restantes_pInfo) {
			indice_arreglo_informacion++;
			palabra_informacion = bloque_informacion[indice_arreglo_informacion];
			bits_restantes_pInfo = 32;
		}

		/*similar a la sentencia de arriba, cuando se completo una palabra del 
		 * bloque codificado, esta se guarda en el arreglo, y se resetea la 
		 * variable para codificar la siguiente
		 */
		if (!bits_restantes_pCod) {
			bloque_codificado[indice_arreglo_codificacion] = palabra_codificada;
			indice_arreglo_codificacion++;
			bits_restantes_pCod = 32;
			palabra_codificada = 0x0;
		}

		/*se añade el nuevo bit al bloque */
		palabra_codificada = palabra_codificada << 1;
		bits_restantes_pCod--;

		/*se comprueba que el 'indice' no sea potencia de 2,
		 * (esto es si posee solo un bit en 1)
		 * lugar que corresponde a un bit de control,
		 * sino, se copia un bit de informacion y se actualiza
		 * los de control*/
		auxiliar = indice; auxiliar--;
		if (indice & auxiliar) {
			auxiliar = palabra_informacion & 0x80000000;	/*se extrae el primer bit (el mas significativo) */
			/*si el bit de info no es cero... */
			if (auxiliar) {
				bits_control = bits_control ^ indice ^ 0x8000;
				palabra_codificada++;
			}

			/*se desplaza la lectura un lugar */
			palabra_informacion = palabra_informacion << 1;
			bits_restantes_pInfo--;
			bits_restantes_total--;
		}
	
		/*se pasa al siguiente indice */
		indice--;
	}

	/*si los bits de entrada con completaron el bloque, se rellena con ceros */
	while (indice) {
		if (bits_restantes_pCod) {
			palabra_codificada = palabra_codificada << bits_restantes_pCod;
			bloque_codificado[indice_arreglo_codificacion] = palabra_codificada;
			indice_arreglo_codificacion++;
			indice -= bits_restantes_pCod;
			bits_restantes_pCod = 0;
		} else {
			bloque_codificado[indice_arreglo_codificacion] = 0x0;
			indice_arreglo_codificacion++;
			indice -= 32;
		}
	}

	/*se copia la ultima palabra, que queda fuera cuando el indice llega a cero */
	/* bloque_codificado[indice_arreglo_codificacion] = palabra_codificada; */


	// 0100 0000 0000 = 0x400
	/*se recorren los bits de control (de mayor a menor),
	 * para ubicarlos en el bloque y para actualizar el bit de paridad total
	 */
	/*DEBUG CAMBIAR DESPUES */
	auxiliar = 0x20;
	while (auxiliar) {
		/*si el bit es 1... */
		if (bits_control & auxiliar) {
			indice_arreglo_codificacion = (auxiliar - 1) / 32;	/*indice del arreglo donde va el bit */
			indice = (auxiliar - 1) % 32;	/* posicion del bit en la palabra (en el orden de su peso) */
			palabra_codificada = 0x1 << indice;		/*mascara del bit de control */
			
			/*se invierte el indice del arreglo,ya que el bloque se ordena de manera invertida*/
			indice_arreglo_codificacion = (TAM_ARREGLO_4096 - 1) - indice_arreglo_codificacion; 
																					

			/*se escribe el bit de control */
			bloque_codificado[indice_arreglo_codificacion] = bloque_codificado[indice_arreglo_codificacion] | palabra_codificada;

			/*se actualiza el bit de paridad total
			 * (0x8000 = primer bit)*/
			bits_control = bits_control ^ 0x8000;

		}
		/** se pasa al siguiente bit de control,
		 * 'auxiliar' funciona como una mascara para extraerlo de 'bits_control',
		 * a su vez tambien numericamente representa su posicion en el bloque codificado
		 * */
		auxiliar = auxiliar >> 1;
	}


	/** se introduce el bit de paridad
	 * (0x80000000 = primer bit de 32)
	 */
	if (bits_control & 0x8000) bloque_codificado[0] = bloque_codificado[0] | 0x80000000;

	return bits_restantes_pInfo;
}


/** 
 *
 * @param bloque_codificado Bloque de 4096 bits a decodificar.
 * @param bits_restantes_pInfo Cantidad de bits de informacion de la primera palabra a procesar.
 * @param bloque_informacion Arreglo destino de la informacion decodificada.
 *
 * @return Devuelve el sindrome del bloque + la correspondencia de paridad en el primer bit.
 */
uint16_t _hamming_decodificar_bloque_4096(uint32_t bloque_codificado[],
		uint8_t bits_restantes_pInfo,
		uint32_t bloque_informacion[]) {
	uint16_t indice = NUM_BITS_TOTAL_4096 - 1,
			 indice_arreglo_cod = 0,
			 indice_arreglo_info = 0,
			 auxiliar,
			 sindrome = 0x0;
	uint32_t palabra_codificada,
			 palabra_informacion = 0x0;
	uint8_t bits_restantes_pCod = 32;

	/*iniciacion */
	palabra_codificada = bloque_codificado[indice_arreglo_cod];
	palabra_informacion = bloque_informacion[indice_arreglo_info];

	/*Se extrae el bit de paridad */
	if (palabra_codificada & 0x80000000) sindrome = 0x8000;
	palabra_codificada = palabra_codificada << 1;
	bits_restantes_pCod--;

	while (indice) {

		if (!bits_restantes_pCod) {
			indice_arreglo_cod++;
			palabra_codificada = bloque_codificado[indice_arreglo_cod];
			bits_restantes_pCod = 32;
		}

		if (!bits_restantes_pInfo) {
			bloque_informacion[indice_arreglo_info] = palabra_informacion;
			indice_arreglo_info++;
			bits_restantes_pInfo = 32;
			palabra_informacion = 0x0; /*asignacion innecesaria */
		}

		/*se controla si el indice NO es una potencia de dos */
		auxiliar = indice; auxiliar--;
		if (indice & auxiliar) {
			palabra_informacion = palabra_informacion << 1;
			bits_restantes_pInfo--;

			/*si primer bit es igual a 1 */
			if (palabra_codificada & 0x80000000) palabra_informacion++;
		}

		if (palabra_codificada & 0x80000000) {
			sindrome = sindrome ^ indice ^ 0x8000;
		}

		palabra_codificada = palabra_codificada << 1;
		bits_restantes_pCod--;

		indice--;
	}

	/*se guarda la ultima palabra incompleta, ya que los bits de informacion
	 * no ocupan toda la ultima palabla como el bloque de salida.
	 * (si los bits restantes fuera igual a 32, entonces la palabra se habria guardado)*/
	if (bits_restantes_pInfo != 32)
		bloque_informacion[indice_arreglo_info] = palabra_informacion;

	return sindrome;
}


/* Funcion privada para codificar unicamente bloques de 8 bits.
 *
 * @param bloque_informacion byte cuyos 8 bits son de informacion a codificar
 * @return devuelve dos bytes codificados con los de informacion ingresados por parametro
 */
uint16_t _hamming_codificar_bloque_8(uint8_t bloque_informacion) {
	uint8_t bits_control,	/* significado de cada bit:
							   b_7 : paridad de primer bloque
							   b_6 : sin uso
							   b_5 : sin uso
							   b_4 : sin uso
							   b_3 : sin uso
							   b_2 : bit de control c2 de 2er bloque
							   b_1 : bit de control c1 de 2er bloque
							   b_0 : bit de control c0 de 2er bloque
		 */
			indice,
			cantidad_bloques_procesados = 0x2,
			auxiliar;

	uint16_t bloques_codificados = 0x0;	// retorno de la funcion


	/*se repite 2 veces, para retornar dos bloques codificados */
	do {
		/*iniciacion de parametros */
		bits_control = 0x0; indice = 0x8; auxiliar = 0x0;


		/* mientras el indice no sea cero */
		while (indice) {
			/* si el indice no es una potencia de 2 */
			auxiliar = indice; auxiliar--;


			/*se pasa al siguiente bit del bloque codificado */
			bloques_codificados = bloques_codificados << 1;

			if (indice & auxiliar) {
				/* se enmascara el primer bit (el mas significativo) */
				auxiliar = bloque_informacion & 0x80;


				// si auxiliar es distinto de cero
				// (esto es si el bit enmascarado es 1) 
				if (auxiliar) {
					bits_control = bits_control ^ (indice | 0x80);	// Xor a bits de control + el de paridad
					bloques_codificados++;	/*se copia el bit en la variable de salida */
				}

				/*se desplaza los bits de informacion */
				bloque_informacion = bloque_informacion << 1;
			};

			/*se incrementa el indice y se decrementa el contador */
			indice--;
		}

		/* Se introducen los bits de control en el bloque */
		/*c0 */
		if (bits_control & 0x1) {
			bloques_codificados = bloques_codificados | 0x1;
			bits_control = bits_control ^ 0x80;
		}
		/*c1 */
		if (bits_control & 0x2) {
			bloques_codificados = bloques_codificados | 0x2;
			bits_control = bits_control ^ 0x80;
		}
		/*c2 */
		if (bits_control & 0x4) {
			bloques_codificados = bloques_codificados | 0x8;
			bits_control = bits_control ^ 0x80;
		}

		/* bit de paridad total */
		bloques_codificados = bloques_codificados | (bits_control & 0x80);

		cantidad_bloques_procesados--;

		} while (cantidad_bloques_procesados);

	return bloques_codificados;
}

/* Decodifica un bloque de Hamming de 8 bits.
 *
 * Los dos bytes devueltos por la funcion corresponden al sindrome de cada bloque.
 * El numero representado por el byte es la posicion donde se encuentra el error en el bloque, excepto por el primer bit (el mas significativo) que si se encuentra en 1 es porque no se corresponde la paridad total con el respectivo bit.
 *
 * @return Devuelve ambos sindromes en un byte cada uno + el bit de paridad total.
 */
uint16_t _hamming_decodificar_bloque_8(uint16_t bloque_codificado, uint8_t *bloque_informacion) {
	uint8_t indice, bloques_procesados = 2;
	uint16_t sindromes = 0x0, auxiliar;
	uint16_t paridad_totales = 0x0;	/*p_1 000 0000
									  p_0 000 0000 */

	while (bloques_procesados) {
		/* se salta el bit de paridad total */
		indice = 7;
		sindromes = sindromes << 8;

		paridad_totales = paridad_totales << 8;
		/* se extrae y almacena el bit de paridad total */
		if (bloque_codificado & 0x8000) paridad_totales = paridad_totales | 0x80;
		bloque_codificado = bloque_codificado << 1;

		while (indice) {

			/*caso que el indice NO sea potencia de dos */
			auxiliar = indice; auxiliar--;
			if (indice & auxiliar) {
				/* esta seccion se encarga de sacar los bits de informacion */

				*bloque_informacion = *bloque_informacion << 1;
				/*si el primer bit es 1 */
				if (bloque_codificado & 0x8000) (*bloque_informacion)++;
			}

			if (bloque_codificado & 0x8000) {
				/* esta seccion se encarga de calcular el sindrome*/

				/* se calcula el sindrome, esto incluye a los bits de informacion como los de control */
				sindromes = sindromes ^ indice;
				/* se comprueba que la paridad total sea correcta */
				paridad_totales = paridad_totales ^ 0x80;
			}

			bloque_codificado = bloque_codificado << 1;
			indice--;
		}
		
		bloques_procesados--;
	}

	return sindromes | paridad_totales;
}

/** Lee un archivo TXT y crea uno de mismo nombre con extension HA1
 * con el contenido codificado por Hamming en bloques de 8 bits.
 * Los punteros a archvos deben estar PREVIAMENTE abiertos.
 *
 * @param fuente Puntero al archivo fuente a codificar.
 * @param destino Puntero al archivo destino.
 *
 * @return Devuelve la cantidad de bytes de informacion leidos de 'fuente'.
 */
int _hamming_codificar_archivo_8bits(FILE *fuente, FILE *destino) {
	char nombre_fuente[128], nombre_destino[128];
	int byte_leido;		/*recibe si la funcion 'fread' ha leido algo del archivo fuente */
	int bytes_leidos;	/* cuenta los bytes de informacion leidos*/
	uint8_t lectura;
	uint16_t escritura;


	while (!feof(fuente)) {
		byte_leido = fread(&lectura, 1, 1, fuente);
		bytes_leidos += byte_leido;		// actualiza la cuenta

		escritura = _hamming_codificar_bloque_8(lectura);

		/*El control es necesario ya que antes de llegar a EOF la funcion 'fread' intenta leer, devolviento 0 elementos leidos */
		if (byte_leido) fwrite(&escritura, 2, 1, destino);
	}

	// devuelve la cantidad de bytes de informacion leidos
	// (igual al tamanio del archivo de entrada)
	return bytes_leidos;
}

/** Corrige un bloque de 8 bits codificado por Hamming, si este posee un solo error.
 * En caso de dos errores la informacion no es modificada.
 *
 * @param informacion Byte de bloque codificado.
 * @param sindromes Dos bytes con los sindromes y paridades de ambas mitades del byte de informacion.
 * @param estado Debe ser un arreglo de dos entradas. Se ubicaran el estado y tipo de error de ambos bloques.
 */
void _hamming_corregir_bloque_8(uint8_t *informacion, uint16_t sindromes, int estado[]) {
	uint16_t paridad, posicion_error, auxiliar, contador_potencias;
	uint8_t estado_izq, estado_der, mascara;

	/*se separan el bit de paridad total y los bits de control del byte de la derecha */
	paridad = sindromes & 0x80; posicion_error = sindromes & 0x7f;
	/*caso que no corresponda la paridad, hay error en el bloque o en el bit de paridad mismo */
	if (paridad) {
		if (posicion_error) {

			/*solo se corrige si la posicion corresponde a un bit de informacion
			 * (si NO es una potencia de dos)*/
			auxiliar = posicion_error;
			if (posicion_error & --auxiliar) {
				/*se necesita restar las posiciones correspondientes a los bits de control */
				contador_potencias = 0x2;
				while ((0x1 << contador_potencias) < posicion_error) contador_potencias++;
				
				mascara = 0x1 << (posicion_error - contador_potencias - 1);
				*informacion = *informacion ^ mascara;
			}
		}
		estado[1] = EST_UN_ERROR;
	}
	/* caso que la paridad corresponda */
	else {
		/*si el sindrome es distinto de cero, hay dos errores */
		if (posicion_error) estado[1] = EST_DOS_ERRORES;
		else estado[1] = EST_SINERROR;
	}

	/*mismo procedimiento pero para el byte de la izquierda */
	sindromes = sindromes >> 8;
	paridad = sindromes & 0x80; posicion_error = sindromes & 0x7f;
	/*caso que no corresponda la paridad, hay error en el bloque o en el bit de paridad mismo */
	if (paridad) {
		if (posicion_error) {

			/*solo se corrige si la posicion corresponde a un bit de informacion
			 * (si NO es una potencia de dos)*/
			auxiliar = posicion_error;
			if (posicion_error & --auxiliar) {
				/*se necesita restar las posiciones correspondientes a los bits de control */
				contador_potencias = 0x1;
				while ((0x1 << contador_potencias) < posicion_error) contador_potencias++;
				
				mascara = 0x1 << (posicion_error - contador_potencias + 3);
				*informacion = *informacion ^ mascara;
			}
		}
		estado[0] = EST_UN_ERROR;
	}
	/* caso que la paridad corresponda */
	else {
		/*si el sindrome es distinto de cero, hay dos errores */
		if (posicion_error) estado[0] = EST_DOS_ERRORES;
		else estado[0] = EST_SINERROR;
	}
}

/**
 *
 * @param nombre_archivo Nombre del archivo a decodificar. No incluir extension.
 * @return Estado de exito de lectura/escritura de archivos.
 */
int _hamming_decodificar_archivo_8bits(char nombre_archivo[]) {
	char nombre_fuente[TAM_CADENAS_NOMBRE],
		nombre_destino_error[TAM_CADENAS_NOMBRE],
		nombre_destino_corregido[TAM_CADENAS_NOMBRE];

	/*se crean los nombres de los archivos */
	strcpy(nombre_fuente, nombre_archivo);
	strcpy(nombre_destino_error, nombre_archivo);
	strcpy(nombre_destino_corregido, nombre_archivo);
	strcat(nombre_fuente, ".ha1");
	strcat(nombre_destino_error, ".de1");
	strcat(nombre_destino_corregido, ".dc1");

	uint16_t lectura, sindromes;
	uint8_t informacion, bytes_leidos;
	int estado[2];

	/* preparacion de archivos para lectura y escritura */
	FILE *archivo_fuente, *archivo_destino_error, *archivo_destino_corregido;

	archivo_fuente = fopen(nombre_fuente, "rb");
	if (archivo_fuente == NULL) return 1;

	archivo_destino_error = fopen(nombre_destino_error, "wb");
	if (archivo_destino_error == NULL) return 1;
	
	archivo_destino_corregido = fopen(nombre_destino_corregido, "wb");
	if (archivo_destino_corregido == NULL) return 1;

	while (!feof(archivo_fuente)) {
		bytes_leidos = fread(&lectura, SIZEOF_UINT16, 1, archivo_fuente);

		/*evitar caso de lectura cero antes del llegar a EOF */
		if (bytes_leidos) {

			sindromes = _hamming_decodificar_bloque_8(lectura, &informacion);

			/*escritura en el archivo sin correccion */
			fwrite(&informacion, SIZEOF_UINT8, 1, archivo_destino_error);

			/* se corrige el bloque si contiene UN SOLO error */
			_hamming_corregir_bloque_8(&informacion, sindromes, estado);

			/*escritura en el archivo corregido */
			fwrite(&informacion, SIZEOF_UINT8, 1, archivo_destino_corregido);
		}
	}

	fclose(archivo_destino_error);
	fclose(archivo_destino_corregido);
	fclose(archivo_fuente);

	return 0;
}

/** Codifica un archivo TXT por Hamming en bloques de 4096 bits en un archivo HA2.
 *
 * @param nombre_archivo Nombre del archivo TXT a codificar. No incluir extension.
 *
 * @return Devuelve 0 si hubo exito en la lectura y escritura de los archivos. Devuelve 1 en caso contrario.
 */
int _hamming_codificar_archivo_4096bits(char nombre_archivo[]) {
	char nombre_fuente[TAM_CADENAS_NOMBRE], nombre_destino[TAM_CADENAS_NOMBRE];
	FILE *fuente, *destino;
	uint32_t bloque_leido[TAM_ARREGLO_4096 + 1], bloque_codificado[TAM_ARREGLO_4096];
	uint32_t bytes_leidos, bits_a_codificar = 0, bytes_a_leer;
	uint8_t bits_sobrantes = 0;

	strcpy(nombre_fuente, nombre_archivo); strcat(nombre_fuente, ".txt");
	strcpy(nombre_destino, nombre_archivo); strcat(nombre_destino, ".ha2");

	fuente = fopen(nombre_fuente, "rb");

	if (fuente == NULL) return 1;	/*error */

	destino = fopen(nombre_destino, "wb+");

	if (destino == NULL) return 1;


	while (!feof(fuente)) {
		
		/*al ir leyendo mas bits de informacion de los que se usan, los sobrantes se van
		 * acumulando, cuando sobra una palabra entera sin usarse, se lee una palabra menos
		 * para compensar estas sobras
		 */
		if (bits_sobrantes == 32) bytes_a_leer = (TAM_ARREGLO_4096 - 1) << 2;	/*multiplicar por 4 */
		else bytes_a_leer = TAM_ARREGLO_4096 << 2; /*multiplicar por 4 */


		/*Se lee de a unidades de byte aunque sea un entero de 32 bits (4 bytes)
		 * porque es mas facil obtener la cantidad de bytes leidos por el retorno
		 * de la funcion 'fread', ademas de ser el resultado el mismo.
		 */
		bytes_leidos = fread(bloque_leido + 1,
				1,
				bytes_a_leer,
				fuente);

		bits_a_codificar = (bytes_leidos << 3) + bits_sobrantes; 	/*multiplicar por 8 */

		if (bits_a_codificar > NUM_BITS_INFO_4096) bits_a_codificar = NUM_BITS_INFO_4096;

		bits_sobrantes = _hamming_codificar_bloque_4096(bloque_leido,
				bits_sobrantes, 
				bits_a_codificar, 
				bloque_codificado);

		if (bits_a_codificar) fwrite(bloque_codificado, SIZEOF_UINT32, 2, destino);
		/* if (bytes_leidos) fwrite(bloque_codificado, sizeof(uint32_t), 128, destino); */

		/*se mueve la ultima palabra al inicio para usar los bits que quedaron sin leer */
		bloque_leido[0] = bloque_leido[TAM_ARREGLO_4096];
			
	}

	fclose(destino);
	fclose(fuente);

	return 0;
}

/** Introduce aleatoriamente un error en un bloque de 8 bits,
 * segun la probabilidad indicada.
 *
 * @param bloque Puntero al bloque codificado por Hamming.
 * @param probabilidad Probabilidad con la que aparecera el error en el bloque. Valor entre 0 y 1.
 *
 * @return Devuelve 1 si se introdujo el error. 0 en caso contrario.
 */
int _hamming_error_en_bloque_8(uint8_t *bloque, float probabilidad) {
	/*genera un numero de punto flotante aleatorio*/
	float azar = (float)rand() / (float)RAND_MAX;
	int error_generado = 0;			// si el azar determina generar error, salida de funcion

	/* se determina si se introducira un error en el bloque en base a 'azar',
	 * si 'azar' es menor que probabilidad, entonces SI se introduce un error*/
	if (azar <= probabilidad) {
		/*se determina al azar el bit a cambiar (cantidad de shifts)*/
		int posicion_bit = rand() % 8;
		uint8_t mascara_error = 0x01 << posicion_bit;		// cantidad de shifts aleatoria
		
		*bloque = (*bloque) ^ mascara_error;
		error_generado = 1;
	}

	return error_generado;
}

/** Introduce de manera aleatoria (a lo sumo) un error en cada bloque de un archivo codificado HA1.
 * Genera un archivo HE1.
 * Recibe por parametro la probabilidad con la que cada bloque recibira un error.
 *
 * @param nombre_fuente Nombre del archivo fuente en el que se introducira errores.
 * @param probabilidad Valor entre 0 y 1. Valor de 1 es probabilidad total.
 * @return Devuelve la cantidad de errores que se introdujo en el archivo. -1 en caso de error.
 */
int _hamming_error_en_archivo_8bits(char nombre_fuente[], float probabilidad) {
	char nombre_destino[TAM_CADENAS_NOMBRE];		// nombre de archivo destino
	nombre_archivo_quitar_extension(nombre_destino, nombre_fuente);	// se quita la extension HA1
	strcat(nombre_destino, ".he1");							// se incluye la extension HE1
	
	FILE *archivo_fuente, *archivo_destino;
	/* apertura y control de archivos */
	archivo_fuente = fopen(nombre_fuente, "rb");
	if (archivo_fuente == NULL) {
		return -1;	// error al abrir el archivo fuente
	}
	archivo_destino = fopen(nombre_destino, "wb+");	// si existe lo sobreescribe
	if (archivo_destino == NULL) {
		return -1;	// error al crear el nuevo archivo
	}

	int cantidad_errores = 0;	// contador de errores introducidos
	int bytes_leidos = 0;	// variable necesaria para el control de lectura
	uint8_t bloque_codificado;

	// recorrido de la fuente bloque por bloque (byte a byte)
	while (!feof(archivo_fuente)) {
		bytes_leidos = fread(&bloque_codificado, 1, 1, archivo_fuente);
		// si hubo lectura de fread
		if (bytes_leidos) {
			// se introduce potencial error en bloque, mas incrementar la cuenta
			cantidad_errores += _hamming_error_en_bloque_8(&bloque_codificado, probabilidad);

			fwrite(&bloque_codificado, 1, 1, archivo_destino);
		}
	}

	fclose(archivo_destino);
	fclose(archivo_fuente);

	return cantidad_errores;
}

/** Remueve la extension del nombre completo de un archivo.
 * El resultado es copiado en otra cadena.
 *
 * @param destino Cadena donde se escribira el resultado.
 * @param fuente Nombre de archivo completo (con extension).
 */
void nombre_archivo_quitar_extension(char destino[], char fuente[]) {
	int caracteres_solo_extension = strlen(strrchr(fuente, '.'));
	int caracteres_solo_nombre = strlen(fuente) - caracteres_solo_extension;

	// copia el nombre sin la extension
	strncpy(destino, fuente, caracteres_solo_nombre);
}
