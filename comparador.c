#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/wait.h>
#include <sys/types.h>
#include "mensaje.h"

// SISTEMAS OPERATIVOS - DIINF USACH - 2/2020
// PROFESOR : CRISTOBAL ACOSTA
// AYUDANTE : MARCELA RIVERA
// AUTORES  : TOMÁS LÓPEZ - CARLOS BARAHONA

int main(int argc, char* argv[]){ 
    // recibo lo que me envió el padre
    mensaje *aviso = (mensaje*)malloc(sizeof(mensaje));
    int x = atoi(argv[1]); // cantidad de caracteres del archivo de entrada
    int y = atoi(argv[2]); // cantidad de caracteres de la secuencia a buscar
    char* nombreArchivo = (char*)malloc(sizeof(char)*(x)); // asigno memoria para almacenar el nombre del archivo
    char* secuencia = (char*)malloc(sizeof(char)*(y)); // asigno memoria para almacenar la secuencia
	read(STDIN_FILENO, aviso, sizeof(mensaje));
    read(STDIN_FILENO, nombreArchivo, sizeof(char)*x);
    read(STDIN_FILENO, secuencia, sizeof(char)*y);
    // genero el nombre del archivo de salida del proceso correspondiente 
    int n = sizeof(aviso->identificador)/sizeof(int); // cantidad de digitos del identificador
    int qCaracteres = 8 + y + n; // cantidad de caracteres del archivo de salida 
    char *salida = (char*)malloc(sizeof(char)*qCaracteres); // asigno memoria al string que tendrá el nombre del archivo de salida 
    char *number = (char*)malloc(sizeof(char)*n); // asigno memoria al string que tendrá el identificador del proceso
    sprintf(number, "%d", aviso->identificador);  
    strcat(salida, "rp_");
    strcat(salida, secuencia);
    strcat(salida, "_");
    strcat(salida, number);
    strcat(salida, ".txt");
    // abro el archivo a leer
    FILE* fpin = fopen(nombreArchivo, "r"); 
    fseek(fpin, aviso->posCursor, SEEK_SET); // muevo el cursor a donde me corresponde empezar a leer
    // creo el archivo de salida 
    FILE* fpout = fopen(salida, "w");
    size_t len = 0;
    char* aux;
    getline(&aux, &len, fpin); // lee linea completa 
    long countL = strlen(aux); // cantidad de caracteres de la linea + salto de linea // sirve para el ciclo donde comparo la secuencia con la linea leída 
    fseek(fpin, -countL, SEEK_CUR); // vuelvo a donde debo empezar a leer 
    countL = countL -1;
    long countS = strlen(secuencia);
    // empiezo a buscar y comparar 
    for (int i = 0; i < aviso->lineas; i++){ // lee linea a linea
        fscanf(fpin, "%s", aux);
        // aqui comparo el string leído con la secuencia
        int acierto = 0; // cantidad de aciertos
        for (int j = 0; j < countL; j++){ // lee caracteres de una linea
            int h = j;
            // lo que falta por comparar no cubre la secuencia completa  
            if (countS > countL-h){
                // ya no se encuentra la cadena así que se escribe que no y se pasa a la siguiente linea (j = countL)
                fprintf(fpout,"%s    NO\n",aux);
                j = countL;
            }
            else{
                for (int  a = 0; a < countS; a++){ // recorro la secuencia desde cada posición de j , de está forma me aseguro de comprobar sí la secuencia está entre medio o superpuesta dentro del string
                    if (secuencia[a] == aux[h]){
                        acierto++;
                        h++;
                        if (acierto == countS){
                        // si se encuentra la secuencia , escribo en el archivo de salida y paso a la siguiente linea (a = countS ^ j = countL) // con una ocurrencia nos basta
                            fprintf(fpout,"%s    SI\n",aux);
                            j = countL;
                            a = countS;
                        }
                    }
                    else{
                        acierto = 0; // reseteo contador de aciertos 
                        a = countS; // vuelvo al incicio de la secuencia 
                    }
                }
            }
        }     
    }
    fclose(fpin);
    fclose(fpout);
    // termina su trabajo 
    return 0;
}