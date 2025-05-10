# Codificador de Hamming
## Compilación

Para compilar el programa se debe usar el siguiente comando (si se realiza en GCC):

```
gcc -Wall -lm Cli_hamming.c Hamming.c -o hamming.exe
```

`-lm` es necesario para vincular la librería *math.h*.

En `-o` se da el nombre/dirección del archivo ejecutable resultante, puede ser cualquiera.

`-Wall` muestra todas las advertencias del compilador. No es necesario para la compilación.

Si desea depurarse el programa con GDB debe agregarse el argumento `-g3`, el cual incluye todos los símbolos de depuración.

## Uso

El programa puede ejecutarse sin argumentos para obtener los comandos disponibles.

**Operaciones disponibles:**

### Codificar
	codificar (8|256|4096) (nombre_archivo)
	"Codifica el archivo con codigos de Hamming en el tamanio de bloque indicado"

### Alterar
	alterar (nombre_archivo) (probabilidad)
	"Introduce errores en el archivo indicado segun 'probabilidad', (valor entre 0 y 1)"

### Decodificar
	decodificar (nombre_archivo)
	"Decodifica el archivo indicado y escribe la informacion en dos
	nuevos archivos, uno sin corregir y el otro corregido"
