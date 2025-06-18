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


// extensiones de cada tipo de formato
const char * ext_strings[] = {
	".txt", ".ha1",".ha2",".ha3",".he1",".he2",".he3",
	".de1", ".de2", ".de3", ".dc1", ".dc2", ".dc3"
};


// ------------------------------------------------------------------
// FUNCIONES PARA TAMAÑOS DE BLOQUE GENERAL
//

/** Codifica la cantidad de bits indicada del buffer 'informacion' cons
 * Hamming en bloques del tipo pasado por parametro.
 *
 * El parametro 'bits_restantes_total' es la cantidad de bits de informacion
 * a tomar del buffer de entrada, el nombre hace alucion a los bits "disponibles".
 *
 * @param ham_tipo Constante de tamaño de bloque de Hamming
 * @param informacion Puntero a buffer con los bits de informacion
 * @param bloque_codificado Puntero a buffer destino donde se ubicara el bloque codificado
 * @param bits_restantes_total Cantidad de bits de informacion a codificar
 *
 * @return Cero si la operacion se realizo correctamente. 1 en caso de error.
 */
int _hamming_codificar_bloque(
		const int ham_tipo,
		struct buffer_bits *informacion,
		struct buffer_bits *bloque_codificado,
		int bits_restantes_total) {

	uint8_t *ptr_buffer_informacion, *ptr_buffer_codificado;
	struct palabra_buffer p_informacion,p_codificada;
	struct sindrome vector_control = {
		.bits_control = 0x0,
		.paridad_total = 0x0
	};

	ptr_buffer_informacion = informacion->palabra_inicio;
	ptr_buffer_codificado = bloque_codificado->palabra_inicio;

	p_informacion.p = *ptr_buffer_informacion;
	p_informacion.bits_restantes = informacion->palabra_inicio_bits;
	p_codificada.p = 0x0;
	p_codificada.bits_restantes = 8;

	// inicializa el indice en base al tamaño de bloque elegido
	int indice = NUM_BITS_TOTAL[ham_tipo];

	// caso: mientras existan bits de informacion, o
	// el indice no sea cero
	while (bits_restantes_total && indice) {

		if (!p_informacion.bits_restantes) {
			++ptr_buffer_informacion;
			p_informacion.p = *ptr_buffer_informacion;
			p_informacion.bits_restantes = 8;
		}

		if (!p_codificada.bits_restantes) {
			*ptr_buffer_codificado = p_codificada.p;
			++ptr_buffer_codificado;

			p_codificada.p = 0x0;
			p_codificada.bits_restantes = 8;
		}

		p_codificada.p = p_codificada.p << 1;
		--(p_codificada.bits_restantes);
	
		uint8_t bit_extraido;
		// si no es potencia de 2 
		if (indice & (indice-1)) {
			bit_extraido = p_informacion.p & 0x80;	// se extrae el primer bit

			// si el bit no es cero
			if (bit_extraido) {
				// calculo de bits de control
				vector_control.bits_control = vector_control.bits_control ^ indice;
				// calculo de paridad total
				vector_control.paridad_total = vector_control.paridad_total ^ 0x80;

				// se agrega el bit a la palabra codificada
				++(p_codificada.p);
			}

			// se desplaza el bit extraido
			p_informacion.p = p_informacion.p << 1;
			--(p_informacion.bits_restantes);

			// se reduce los bits restantes de informacion a codificar
			--bits_restantes_total;
		}

		// se pasa al siguiente indice
		--indice;
	}

	// se actualiza en el buffer la ultima palabra utilizada
	*ptr_buffer_informacion = p_informacion.p;
	informacion->palabra_ultima = ptr_buffer_informacion;
	informacion->palabra_ultima_bits = p_informacion.bits_restantes;


	// si los bits de info se acabaron, se completa el resto con ceros
	while (indice > 0 && ptr_buffer_codificado - bloque_codificado->arreglo
				< (bloque_codificado->tam_arreglo)) {
		// si quedaban bits disponibles en la palabra codificada
		if (p_codificada.bits_restantes) {
			p_codificada.p = p_codificada.p << p_codificada.bits_restantes;
			indice -= p_codificada.bits_restantes;
			p_codificada.bits_restantes = 0;

			*ptr_buffer_codificado = p_codificada.p;
			++ptr_buffer_codificado;
		} else {
			// se completa el resto del bloque codificado con ceros
			// caso: el puntero no llega al final del buffer
			*ptr_buffer_codificado = 0x0;
			++ptr_buffer_codificado;
			indice -= 8;
		}
	}

	// posible caso de error
	// (si todo anda bien no deberia pasar, pero no esta de mas)
	if (indice != 0) return 1;


	// proceso de incluir los bits de control al bloque.
	uint16_t mascara_bit_control = 0x1 << (NUM_BITS_CONTROL[ham_tipo] - 1);
	// se agregan los bits de control al bloque.
	// caso: mientras la mascara no sea cero
	while (mascara_bit_control) {
		// caso: si el bit de control es 1
		if (vector_control.bits_control & mascara_bit_control) {
			// indice arreglo: en que palabra del buffer va el bit
			int indice_arreglo = (mascara_bit_control-1) / 8;
			indice_arreglo = (bloque_codificado->tam_arreglo - 1) - indice_arreglo;	// se invierte

			// posicion palabra: la posicion del bit en la palabra
			int posicion_palabra = (mascara_bit_control-1) % 8;
			uint8_t bit_control = 0x1 << posicion_palabra;

			// se agrega el bit en el bloque codificado
			ptr_buffer_codificado = bloque_codificado->arreglo + indice_arreglo;
			*ptr_buffer_codificado = *ptr_buffer_codificado | bit_control;
			
			// se actualiza el bit de paridad total
			vector_control.paridad_total = vector_control.paridad_total ^ 0x80;
		}

		// se pasa al siguiente bit de control
		mascara_bit_control = mascara_bit_control >> 1;
	}

	// por ultimo, se introdude el bit de paridad total,
	// el cual va al inicio
	ptr_buffer_codificado = bloque_codificado->arreglo;	// indice 0
	*ptr_buffer_codificado = *ptr_buffer_codificado | vector_control.paridad_total;

	// retorna 0 si la operacion se realizo correctamente
	return 0;
}

/** 
 * @param ham_tipo Tamaño de bloque a utilizar en la codificación.
 * @param nombre_fuente Nombre de archivo TXT a codificar. (Incluir extensión).
 *
 * @return Cantidad de bloques resultantes. -1 en caso de error.
 */
int _hamming_codificar_archivo(const int ham_tipo, char nombre_fuente[]) {
	char nombre_destino[TAM_CADENAS_NOMBRE];
	FILE *fuente, *destino;

	// se determina el tipo de extension.
	// como solo hay dos tipos de extension se usa un 'if',
	// si huviese mas habria que usar un 'switch'.
	const char *ext_destino = ham_tipo == HAM256 ?
		ext_strings[HA2] : ext_strings[HA3];

	// definicion nombre de archivo destino
	nombre_archivo_quitar_extension(nombre_destino, nombre_fuente);
	strcat(nombre_destino, ext_destino);

	// apertura de archivos
	fuente = fopen(nombre_fuente, "rb");
	if (fuente == NULL) return -1;
	destino = fopen(nombre_destino, "wb");
	if (destino == NULL) return -1;

	// iniciacion de buffers
	struct buffer_bits informacion_fuente = {
		.tam_arreglo = TAM_ARREGLO[ham_tipo] + 2
	}, codificacion_destino = {
		.tam_arreglo = TAM_ARREGLO[ham_tipo]
	};
	buffer_bits_init(&informacion_fuente);
	buffer_bits_init(&codificacion_destino);
	informacion_fuente.palabra_inicio = informacion_fuente.arreglo + 1;
	informacion_fuente.palabra_inicio_bits = 8;
	uint8_t *ptr_informacion_fuente = informacion_fuente.arreglo + 1;


	// cabecera del archivo
	uint32_t cabecera[2] = {0,0};	// [0] contador de bloques escritos
									// [1] cantidad de bits de info en ultimo bloque
	{	// control de escritura correcta
		int cabecera_escrita = 
			fwrite(&cabecera, SIZEOF_UINT32, 2, destino);
		if (cabecera_escrita != 2) return -1;
	}

	// recorrido del archivo a codificar
	int bytes_leidos = 0, bytes_a_leer, bits_a_codificar;
	while (!feof(fuente)) {
		// si sobraron bits del bloque codificado anteriormente
		if (informacion_fuente.palabra_ultima_bits) {
			informacion_fuente.palabra_inicio_bits = informacion_fuente.palabra_ultima_bits;
			*(informacion_fuente.arreglo) = *(informacion_fuente.palabra_ultima);
			informacion_fuente.palabra_inicio = informacion_fuente.arreglo;
		} else {
			informacion_fuente.palabra_inicio = informacion_fuente.arreglo + 1;
			informacion_fuente.palabra_inicio_bits = 8;
		}

		// calculo de bytes a leer segun bits sobrantes
		{
			double bytes_necesarios =
				(double)(NUM_BITS_INFO[ham_tipo] - informacion_fuente.palabra_ultima_bits) / 8;
			bytes_a_leer = (int) ceil(bytes_necesarios);
		}
		
		//TERMINAR
		// lectura del archivo
		bytes_leidos = fread(ptr_informacion_fuente, 1, bytes_a_leer, fuente);

		// se determinan la cantidad de bits de info que contendra el bloque
		bits_a_codificar = (bytes_leidos * 8) + informacion_fuente.palabra_ultima_bits;
		if (bits_a_codificar > NUM_BITS_INFO[ham_tipo]) bits_a_codificar = NUM_BITS_INFO[ham_tipo];

		// caso: hay bits para codificar
		if (bits_a_codificar) {
			_hamming_codificar_bloque(
					ham_tipo,
					&informacion_fuente,
					&codificacion_destino,
					bits_a_codificar);

			// se escribe en el archivo
			fwrite(codificacion_destino.palabra_inicio, 1, codificacion_destino.tam_arreglo, destino);
			// se actualiza la cabecera del archivo destino
			++(cabecera[0]);	// cantidad de bloques codificados
			cabecera[1] = bits_a_codificar;
		}
	}

	fclose(fuente);

	rewind(destino);
	fwrite(&cabecera, SIZEOF_UINT32, 2, destino);
	
	fclose(destino);

	buffer_bits_free(&informacion_fuente);
	buffer_bits_free(&codificacion_destino);

	return cabecera[0];
}

struct sindrome _hamming_decodificar_bloque(
		const int ham_tipo,
		struct buffer_bits *bloque_codificado,
		struct buffer_bits *informacion) {
	
	// cursor de buffers
	uint8_t *ptr_buffer_codificado = bloque_codificado->palabra_inicio,
		*ptr_informacion = informacion->palabra_inicio;

	// sindrome del bloque
	struct sindrome sindrome = {
		.bits_control = 0x0
	};

	// palabras donde se trabajara
	struct palabra_buffer p_codificada = {
		.p = *(bloque_codificado->palabra_inicio),
		.bits_restantes = 8
	}, p_informacion = {
		.p = *(informacion->palabra_inicio),
		.bits_restantes = informacion->palabra_inicio_bits
	};

	// se extrae el bit de paridad total
	sindrome.paridad_total = p_codificada.p & 0x80;
	p_codificada.p = p_codificada.p << 1;
	--(p_codificada.bits_restantes);


	uint16_t indice = NUM_BITS_TOTAL[ham_tipo] - 1;	// -1 porque ya se extrajo la paridad
	uint8_t bit_p_codificada;
	while (indice) {

		if (!p_codificada.bits_restantes) {
			++ptr_buffer_codificado;
			p_codificada.p = *ptr_buffer_codificado;
			p_codificada.bits_restantes = 8;
		}

		if (!p_informacion.bits_restantes) {
			*ptr_informacion = p_informacion.p;
			++ptr_informacion;
			p_informacion.p = 0x0;
			p_informacion.bits_restantes = 8;
		}

		// se extrae el bit en el extremo de la palabra codificada
		bit_p_codificada = p_codificada.p & 0x80;

		// caso: si el indice no es potencia de 2
		if ((indice -1) & indice) {
			// se desplaza la palabra de informacion
			p_informacion.p = p_informacion.p << 1;
			--(p_informacion.bits_restantes);

			// si el bit es igual a 1
			if (bit_p_codificada) ++(p_informacion.p);		// se introduce el bit
		}

		// se calcula el sindrome mientras se recorre el bloque
		if (bit_p_codificada) {
			sindrome.bits_control = sindrome.bits_control ^ indice;
			sindrome.paridad_total = sindrome.paridad_total ^ 0x80;
		}

		// se desplaza la palabra del bloque
		p_codificada.p = p_codificada.p << 1;
		--(p_codificada.bits_restantes);

		--indice;
	}

	// se guarda la ultima palabra de informacion de nuevo en el buffer,
	// ya que pueden sobrar bits a usar en el siguiente bloque
	*ptr_informacion = p_informacion.p;
	informacion->palabra_ultima = ptr_informacion;
	informacion->palabra_ultima_bits = p_informacion.bits_restantes;

	return sindrome;
}


/** Crea nuevos archivos con la informacion decodificada por Hamming en el archivo
 * fuente indicado.
 *
 * @param ham_tipo Tamaño de bloque de Hamming.
 * @param nombre_fuente Nombre de archivo fuente a decodificar.
 *
 * @return 0 si se realizo la decodificacion correctamente. -1 si hubo algun error.
 * 1 si algun bloque tenia un error. 2 si algun bloque tenia dos errores.
 */
int _hamming_decodificar_archivo(const int ham_tipo, char nombre_fuente[]) {
	int retorno_nivel_error = EST_SINERROR;
	char nombre_destino_error[TAM_CADENAS_NOMBRE],
		nombre_destino_corregido[TAM_CADENAS_NOMBRE];
	FILE *fuente, *destino_error, *destino_corregido;
	const char *ext_error = ham_tipo == HAM256 ?
							ext_strings[DE2] : ext_strings[DE3],
		*ext_corregido = ham_tipo == HAM256 ?
							ext_strings[DC2] : ext_strings[DC3];

	nombre_archivo_quitar_extension(nombre_destino_error, nombre_fuente);
	nombre_archivo_quitar_extension(nombre_destino_corregido, nombre_fuente);
	strcat(nombre_destino_error, ext_error);
	strcat(nombre_destino_corregido, ext_corregido);


	struct buffer_bits bloque_codificado = {
		.tam_arreglo = TAM_ARREGLO[ham_tipo]
	}, informacion = {
		.tam_arreglo = TAM_ARREGLO[ham_tipo] + 1
	};
	buffer_bits_init(&bloque_codificado);
	buffer_bits_init(&informacion);
	informacion.palabra_inicio = informacion.arreglo + 1;
	informacion.palabra_inicio_bits = 8;

	fuente = fopen(nombre_fuente, "rb");
	if (fuente == NULL) return -1;
	destino_error = fopen(nombre_destino_error, "wb");
	if (destino_error == NULL) return -1;
	destino_corregido = fopen(nombre_destino_corregido, "wb");
	if (destino_corregido == NULL) return -1;

	uint32_t cantidad_bloques, bits_info_ultimo_bloque;
	{
		uint32_t cabecera_fuente[2];
		int palabras_leidas = 
			fread(&cabecera_fuente, SIZEOF_UINT32, 2, fuente);
		if (palabras_leidas != 2) return -1;
		cantidad_bloques = cabecera_fuente[0];
		bits_info_ultimo_bloque = cabecera_fuente[1];
	}

	
	int bytes_leidos, bytes_a_escribir;
	struct sindrome sindrome;
	while (!feof(fuente)) {
		bytes_leidos = 
			fread(bloque_codificado.palabra_inicio, 1, TAM_ARREGLO[ham_tipo], fuente);

		if (bytes_leidos) {
			sindrome =
				_hamming_decodificar_bloque(ham_tipo, &bloque_codificado, &informacion);


			uint8_t *inicio, *fin;
			inicio = informacion.palabra_inicio;
			if (cantidad_bloques != 1) {
				fin = informacion.palabra_ultima_bits == 0 ?
					informacion.palabra_ultima : informacion.palabra_ultima - 1;
				bytes_a_escribir = fin - inicio + 1;

			} else {
				bytes_a_escribir =
					(int) ceil(((double) bits_info_ultimo_bloque) / 8);
			};
			// escritura de informacion SIN corregir
			fwrite(inicio, 1, bytes_a_escribir, destino_error);

			// correccion de informacion
			{
				int tipo_error =
					_hamming_corregir_info(ham_tipo, &informacion, sindrome);

				//se actualiza el tipo de error mas alto
				retorno_nivel_error = tipo_error > retorno_nivel_error ?
					tipo_error : retorno_nivel_error;
			}


			// escritura de informacion CORREGIDA
			fwrite(inicio, 1, bytes_a_escribir, destino_corregido);

			if (informacion.palabra_ultima_bits) {
				*(informacion.arreglo) = *(informacion.palabra_ultima);
				informacion.palabra_inicio = informacion.arreglo;
				informacion.palabra_inicio_bits = informacion.palabra_ultima_bits;

			} else {
				informacion.palabra_inicio = informacion.arreglo + 1;
				informacion.palabra_inicio_bits = 8;
			}

			--cantidad_bloques;
		}
	}

	buffer_bits_free(&bloque_codificado);
	buffer_bits_free(&informacion);

	fclose(fuente);
	fclose(destino_error);
	fclose(destino_corregido);

	return retorno_nivel_error;
}

int _hamming_corregir_info(
		const int ham_tipo,
		struct buffer_bits *informacion,
		struct sindrome sindrome) {

	// caso: no hay error en el bloque
	if (!sindrome.bits_control && !sindrome.paridad_total) {
		return EST_SINERROR;
	}
	// caso: un solo error
	else if (sindrome.bits_control && sindrome.paridad_total) {

		// se corrige solamente si el sindrome NO es una potencia de 2,
		// ya que el error corresponderia a un bit de control
		if ((sindrome.bits_control - 1) & sindrome.bits_control) {

			// se obtiene el indice del error pero en relacion a los bits de info
			uint16_t bit_error_indice =
				indice_restar_bits_control(sindrome.bits_control);

			// se desplaza para acomodar la posicion con el buffer
			bit_error_indice += informacion->palabra_ultima_bits;

			const int indice_arreglo = (bit_error_indice - 1) / 8;

			// los bytes se escriben de derecha a izquierda,
			// cuando el error se encuentra en el ultimo byte en el arreglo,
			// los bits desplazados se encuentran a la izquierda, entonces solo 
			// en este caso no se suman los bits de dezplazamiento para la mascara
			// (se vuelven a restar)
			if (!indice_arreglo) bit_error_indice -= informacion->palabra_ultima_bits;

			const uint8_t mascara_bit = 
				0x1 << ((bit_error_indice - 1) % 8);

			// correccion
			uint8_t *ptr_informacion = informacion->palabra_ultima;
			ptr_informacion -= indice_arreglo;
			*ptr_informacion = *ptr_informacion ^ mascara_bit;
		}

		return EST_UN_ERROR;
	}
	// caso: dos errores en el bloque, no es posible corregir
	else return EST_DOS_ERRORES;
}

int _hamming_error_en_bloque(
		const int ham_tipo,
		struct buffer_bits *bloque,
		float probabilidad) {
	float azar = (float)rand() / (float)RAND_MAX;
	int error_generado = 0;

	if (azar <= probabilidad) {
		int posicion_bit = rand() % 8;
		uint8_t mascara_error = 0x1 << posicion_bit;

		int indice_arreglo = rand() % TAM_ARREGLO[ham_tipo];
		uint8_t *ptr_bloque = bloque->palabra_inicio + indice_arreglo;

		*ptr_bloque = *ptr_bloque ^ mascara_error;

		error_generado = 1;
	}

	return error_generado;
}

int _hamming_error_en_archivo(
		const int ham_tipo,
		char nombre_fuente[],
		float probabilidad) {
	char nombre_destino[TAM_CADENAS_NOMBRE];
	// se determina la extension segun el tipo de codificacion
	const char *ext_destino = ham_tipo == HAM256 ?
							ext_strings[HE2] : ext_strings[HE3];

	nombre_archivo_quitar_extension(nombre_destino, nombre_fuente);
	strcat(nombre_destino, ext_destino);

	FILE *fuente, *destino;
	int contador_errores = 0;

	fuente = fopen(nombre_fuente, "rb");
	if (fuente == NULL) return -1;
	destino = fopen(nombre_destino, "wb");
	if (destino == NULL) return -1;

	struct buffer_bits bloque = {
		.tam_arreglo = TAM_ARREGLO[ham_tipo]
	};
	buffer_bits_init(&bloque);

	// copiar cabecera
	{
		uint32_t cabecera[2];
		fread(cabecera, SIZEOF_UINT32, 2, fuente);
		fwrite(cabecera, SIZEOF_UINT32, 2, destino);
	}

	int bytes_leidos;
	while (!feof(fuente)) {
		bytes_leidos = fread(bloque.palabra_inicio, 1, bloque.tam_arreglo, fuente);

		if (bytes_leidos) {
			contador_errores +=
				_hamming_error_en_bloque(ham_tipo, &bloque, probabilidad);

			fwrite(bloque.palabra_inicio, 1, bloque.tam_arreglo, destino);
		}
	}	

	buffer_bits_free(&bloque);
	fclose(destino);
	fclose(fuente);

	return contador_errores;
}


// ------------------------------------------------------------------
// FUNCIONES ESPECIFICAS PARA BLOQUES DE 8 BITS
//


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
 * El numero representado por el byte es la posicion donde se encuentra el error en el bloque,
 * excepto por el primer bit (el mas significativo) que si se encuentra en 1 es porque no se
 * corresponde la paridad total con el respectivo bit.
 *
 * @param bloque_codificado Dos bloques codificados consecutivos de 8 bits.
 * @param bloque_informacion Puntero a byte donde se escribira la informacion extraida de ambos bloques.
 * @return Devuelve los sindromes de ambos bloques en un byte cada uno + el bit de paridad total.
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
 * Nota: no se realiza control sobre el formato del archivo.
 *
 * @param nombre_fuente Nombre de archivo TXT a codificar.
 *
 * @return Devuelve la cantidad de bytes de informacion leidos de 'fuente'. -1 en caso de error.
 */
int _hamming_codificar_archivo_8(char nombre_fuente[]) {
	char nombre_destino[TAM_CADENAS_NOMBRE];
	FILE *fuente, *destino;
	int byte_leido;		/*recibe si la funcion 'fread' ha leido algo del archivo fuente */
	int bloques_escritos = 0;	/* cuenta los bloques escritos en destino*/
	uint8_t lectura;
	uint16_t escritura;

	/*declaracion y apertura de archivos*/
	//captura del nombre del archivo para destino (debe ser un TXT)
	nombre_archivo_quitar_extension(nombre_destino, nombre_fuente);
	strcat(nombre_destino, ".ha1");		// se agrega la extension
	
	// apertura de archivos.
	// en caso de error la funcion termina, retornando -1
	fuente = fopen(nombre_fuente, "rb");
	if (fuente == NULL) return -1;
	destino = fopen(nombre_destino, "wb");
	if (destino == NULL) return -1;


	while (!feof(fuente)) {
		byte_leido = fread(&lectura, 1, 1, fuente);

		escritura = _hamming_codificar_bloque_8(lectura);

		/*El control es necesario ya que antes de llegar a EOF la funcion 'fread' intenta leer, devolviento 0 elementos leidos */
		if (byte_leido) {
			fwrite(&escritura, 2, 1, destino);
			++bloques_escritos;
		}
	}

	fclose(destino);
	fclose(fuente);
	
	// se escriben de a dos bloques por vez en destino,
	// por lo tanto se acomoda la cantidad
	bloques_escritos *= 2;
	// devuelve la cantidad de bloques escritos
	return bloques_escritos;
}

/** Corrige dos bloques consecutivos de 8 bits (uint16_t) codificado por Hamming, si este posee un solo error.
 * En caso de dos errores la informacion no es modificada.
 *
 * @param bloque Bytea de bloquea codificado.
 * @param sindromes Dos bytes con los sindromes y paridades de ambos bloques.
 * @param estado Debe ser un arreglo de dos entradas. Se ubicaran el estado y tipo de error de ambos bloques.
 */
void _hamming_corregir_bloque_8(uint16_t *bloques, uint16_t sindromes, int estado[]) {
	uint16_t paridad_izq, paridad_der,
			 pos_error_izq, pos_error_der;

	/*se separan el bit de paridad total y los bits de control */
	//derecha
	paridad_der = sindromes & 0x80; pos_error_der = sindromes & 0x7f;
	// izquierda
	sindromes = sindromes >> 8;
	paridad_izq = sindromes & 0x80; pos_error_izq = sindromes & 0x7f;

	// BLOQUE DERECHO
	// ---------------------------------------------
	/* si el sindrome NO es cero */
	if (pos_error_der) {
		/* - 0x100 porque el xor se realiza en el bloque derecho de los 16 bits de 'bloques'.
		 * - se disminuye la posicion en 1 porque si este fuese el primer bit no se realiza shift.
		 * */
				*bloques = (*bloques) ^ ((0x1 << --pos_error_der));	
	} else {
		/* si la paridad total no corresponde, y el sindrome es cero,
		 * caso cuando el error se produce en el bit de paridad */
		if (!paridad_der)
			*bloques = (*bloques) ^ 0x80;
	}
	/* se indica el tipo de error */
	// caso: si hubo algun error
	if (pos_error_der || paridad_der) {
		// caso: si el sindrome Y el bit de paridad indican error
		if (pos_error_der && paridad_der) estado[1] = EST_UN_ERROR;
		else estado[1] = EST_DOS_ERRORES;
	// caso: sin errores
	} else estado[1] = EST_SINERROR;

	// BLOQUE IZQUIERDO
	// ---------------------------------------------
	/* si el sindrome NO es cero */
	if (pos_error_izq) {
		/* - 0x100 porque el xor se realiza en el bloque derecho de los 16 bits de 'bloques'.
		 * - se disminuye la posicion en 1 porque si este fuese el primer bit no se realiza shift.
		 * */
				*bloques = (*bloques) ^ ((0x100 << --pos_error_izq));	
	} else {
		/* si la paridad total no corresponde, y el sindrome es cero,
		 * caso cuando el error se produce en el bit de paridad */
		if (!paridad_izq)
			*bloques = (*bloques) ^ 0x8000;
	}
	/* se indica el tipo de error */
	// caso: si hubo algun error
	if (pos_error_izq || paridad_izq) {
		// caso: si el sindrome Y el bit de paridad indican error
		if (pos_error_izq && paridad_izq) estado[0] = EST_UN_ERROR;
		else estado[0] = EST_DOS_ERRORES;
	// caso: sin errores
	} else estado[0] = EST_SINERROR;
}

/** Decodifica un archivo HA1 o HE1, creando dos archivos DE1 y DC1 sin corregir y
 * corregido respectivamente.
 *
 * @param nombre_fuente Nombre del archivo a decodificar (extension HA1 o HE1).
 * @return Estado de exito de lectura/escritura de archivos. 0 sin errores, 1 caso contrario.
 */
int _hamming_decodificar_archivo_8(char nombre_fuente[]) {
	char nombre_destino_error[TAM_CADENAS_NOMBRE],
		nombre_destino_corregido[TAM_CADENAS_NOMBRE];

	/*se crean los nombres de los archivos */
	nombre_archivo_quitar_extension(nombre_destino_error, nombre_fuente);
	nombre_archivo_quitar_extension(nombre_destino_corregido, nombre_fuente);
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

			// extraccion de bits de informacion + sindrome
			sindromes = _hamming_decodificar_bloque_8(lectura, &informacion);

			/*escritura en el archivo sin correccion */
			fwrite(&informacion, SIZEOF_UINT8, 1, archivo_destino_error);

			/* se corrige el bloque si contiene UN SOLO error */
			_hamming_corregir_bloque_8(&lectura, sindromes, estado);
			/* se extrae los bits de informacion corregidos */
			sindromes = _hamming_decodificar_bloque_8(lectura, &informacion);

			/*escritura en el archivo corregido */
			fwrite(&informacion, SIZEOF_UINT8, 1, archivo_destino_corregido);
		}
	}

	fclose(archivo_destino_error);
	fclose(archivo_destino_corregido);
	fclose(archivo_fuente);

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
int _hamming_error_en_archivo_8(char nombre_fuente[], float probabilidad) {
	char nombre_destino[TAM_CADENAS_NOMBRE];		// nombre de archivo destino
	nombre_archivo_quitar_extension(nombre_destino, nombre_fuente);	// se quita la extension HA1
	strcat(nombre_destino, ".he1");							// se incluye la extension HE1
	
	FILE *archivo_fuente, *archivo_destino;
	/* apertura y control de archivos */
	archivo_fuente = fopen(nombre_fuente, "rb");
	if (archivo_fuente == NULL) {
		return -1;	// error al abrir el archivo fuente
	}
	archivo_destino = fopen(nombre_destino, "wb");	// si existe lo sobreescribe
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

// ------------------------------------------------------------------
// FUNCIONES ADICIONALES
//

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
	destino[caracteres_solo_nombre] = '\0';
}

int tipo_error(struct sindrome sindrome) {
	// caso: no hay error
	if (!sindrome.bits_control && !sindrome.paridad_total) {
		return EST_SINERROR;
	} else {
		// caso: un error
		if (sindrome.bits_control && sindrome.paridad_total) {
			return EST_UN_ERROR;
		} else {
			// caso: dos errores
			return EST_DOS_ERRORES;
		}
	}
}

uint16_t indice_restar_bits_control(uint16_t bits_control_sindrome) {
	uint16_t mascara = 0x8000;

	// caso: mientras la mascara no sea cero
	while (mascara) {
		// si el indice (el sindrome) incluye la posicion de un bit de control
		if (bits_control_sindrome >= mascara) --bits_control_sindrome;

		mascara = mascara >> 1;
	}

	return bits_control_sindrome;
}

/** Devuelve el tipo de extension que contiene el nombre de archivo indicado.
 *
 * @param nombre_archivo Nombre de archivo (solo ha1,ha2,ha3,he1,he2,he3).
 * @return Enumerado de tipo 'extension'. NONE si no es una extension admitida.
 */
int tipo_ext_nombre_archivo(char nombre_archivo[]) {
	char *ext_char = strrchr(nombre_archivo, '.');	// puntero a solamente la extension

	for (int i = 0; i<=DC3; i++) {
		if (strcmp(ext_char, ext_strings[i]) == 0)
			return i;
	}

	return NONE;
}

void buffer_bits_init(struct buffer_bits *buffer) {
	buffer->arreglo =
		(uint8_t *) malloc(sizeof(uint8_t) *
				buffer->tam_arreglo);

	buffer->palabra_inicio = buffer->arreglo;
	buffer->palabra_inicio_bits = 0;

	buffer->palabra_ultima = buffer->arreglo + (buffer->tam_arreglo - 1);
	buffer->palabra_ultima_bits = 0;
}

void buffer_bits_free(struct buffer_bits *buffer) {
	free((void *) buffer->arreglo);

	buffer->palabra_ultima = NULL;
	buffer->palabra_inicio = NULL;
	buffer->palabra_ultima_bits = 0;
	buffer->palabra_inicio_bits = 0;
}


