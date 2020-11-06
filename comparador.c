#include <unistd.h> //Para utilizar fork(), pipes(), entre otros
#include <stdio.h> //Funciones de entrada y salida como printf
#include <stdlib.h> //Asignación de memoria, atoi, etc.
#include <string.h> 
#include <sys/wait.h> //Define las constantes simbólicas para usar con waitpid(), wait() por ejemplo
#include <sys/types.h> //define varios tipos de datos como pid_t

typedef struct mensaje{
    int identificador;
    int lineas;
    int  posCursor;
}mensaje;

int main(int argc, char* argv[]){ 
    mensaje *aviso = (mensaje*)malloc(sizeof(mensaje));
    int x = atoi(argv[1]);
    int y = atoi(argv[2]);
    char* nombreArchivo = (char*)malloc(sizeof(char)*(x));
    char* secuencia = (char*)malloc(sizeof(char)*(y));
	read(STDIN_FILENO, aviso, sizeof(mensaje));
    read(STDIN_FILENO, nombreArchivo, sizeof(char)*x);
    read(STDIN_FILENO, secuencia, sizeof(char)*y);
	printf("soy el hijo: %d\n", aviso->identificador);
    printf("debo leer desde la posicion del cursor: %d\n", aviso->posCursor);
    printf("debo leer %d lineas\n", aviso->lineas);
    printf("el nombre del archivo es: %s\n",nombreArchivo);
    printf("la secuencia a comparar es: %s\n",secuencia);
    return 0;
}