#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "mensaje.h"

#define LECTURA 0
#define ESCRITURA 1

// SISTEMAS OPERATIVOS - DIINF USACH - 2/2020
// PROFESOR : CRISTOBAL ACOSTA
// AYUDANTE : MARCELA RIVERA
// AUTORES  : TOMÁS LÓPEZ - CARLOS BARAHONA

int main(int argc, char** argv){ // argc indica cantidad de argumentos y argv es un arreglo de caracteres que tiene los parametros de entrada por consola 
    char* nameFile = NULL; // nombre del archivo de entrada
    int qProcesses, qLines; // cantidad de procesos a generar y cantidad de lineas a leer del archivo de entrada 
    char* chain = NULL; // string que contiene la secuencia a buscar
    int dflag = 0; // bandera que indica si se muestra el resultado final por pantalla
    int c, largo, i; // variables auxiliares 
    opterr = 0;
      while ((c = getopt (argc, argv, "i:n:c:p:d")) != -1){ //getopt guarda los argumentos ingresados por consola y los guarda en c hasta que no quede ninguno
        switch (c){       
        case 'i':
            nameFile = optarg; // comprobar que el archivo de entrada tenga extensión .txt
            largo = strlen(nameFile);
            i = largo -4;
            int j = 0;
            char extension[4] = ".txt";
            while (i < largo){
                if (extension[j] == nameFile[i]){
                    i++;
                    j++;
                }
                else{
                    fprintf(stderr, "el archivo de entrada no tiene la extension .txt");
                    return 1;
                }
            }        
            break;

        case 'n':
            // convierto la entrada recibida como string a entero // verificar que sea un numero mayor a 0
            if ((qProcesses = atoi(optarg)) > 0){
                break;
            }
            else{
                fprintf(stderr, "la opcion n debe ser un entero positivo\n");
                return 1;
            }

        case 'c':
            // convierto la entrada recibida como string a entero // verificar que sea un numero mayor a 0
            if ((qLines = atoi(optarg)) > 0){ 
                break;
            }
            else{
                fprintf(stderr, "la opcion c debe ser un entero positivo\n");
                return 1;
            }

        case 'p':
            chain = optarg; // comprobar que la secuencia sea válida 
            largo = strlen(chain);
            i = 0;
            while (i < largo){        
                if (chain[i] == 'A' || chain[i] == 'T' || chain[i] == 'C' || chain[i] == 'G'){
                    i++;
                }
                else{
                    fprintf(stderr, "la secuencia contiene caracteres invalidos, las bases nitrogenadas son: A, T, C y G\n");
                    return 1;
                }
            }
            break;

        case 'd':
            dflag = 1;
            break;

        case '?':
            if (optopt == 'c'){
                fprintf(stderr, "Opcion -%c requiere un argumento.\n", optopt);
            }

            else if (isprint(optopt)){
                fprintf(stderr, "Opcion desconocida `-%c' .\n", optopt);
            }

            else{
                fprintf(stderr, "Opcion con caracter desconocido `\\x%x'.\n", optopt);
            }    
            return 1;

        default:
            abort();
        }
    }
    // abrimos el archivo para leer
    FILE *fp = fopen(nameFile, "r");
    if (!fp){
        printf("ERROR : No existe el archivo ingresado\n");
        return 1;
    }
    // obtengo longitud de la linea del archivo
    size_t len = 0;
    char* aux = NULL;
    getline(&aux, &len, fp); // lee una linea completa
    long count = ftell(fp); // obtengo cantidad de caracteres de la linea leída + salto de linea
    rewind(fp); // vuelvo el cursor al inicio
    // lineas que lee cada proceso hijo
    int cantidadLineas = qLines / qProcesses;
    int numProcess;
    // CREAR MULTIPLES PIPES
    // creamos una matriz donde cada fila será un pipe de un proceso hijo 
    int** pipes = (int**)malloc(sizeof(int*)*qProcesses);
    for (numProcess = 0; numProcess < qProcesses; numProcess++){
        pipes[numProcess] = (int*)malloc(sizeof(int)*2);
        pipe(pipes[numProcess]); // inicializamos los pipes
    }
    // CREAR MULTIPLES HIJOS
    int status;
	pid_t pid;
    mensaje *new = (mensaje*)malloc(sizeof(mensaje));
    // parametros por argv que se le enviarán a los hijos (comparador)
    int x = strlen(nameFile); // cantidad de carácteres del archivo de entrada
    int y = strlen(chain); // cantidad de carácteres de la cadena
    char nameF[x];
    sprintf(nameF, "%d", x); // guardo el num como string
    char nameS[y];
    sprintf(nameS, "%d", y);
    //mensaje *aviso = (mensaje*)malloc(sizeof(mensaje));
    new->posCursor = 0;
	for (numProcess = 0; numProcess < qProcesses ; numProcess++){
        new->identificador = numProcess;
        new->lineas = cantidadLineas;
        new->posCursor = numProcess*(cantidadLineas*count); //
        if (qLines % qProcesses != 0 && numProcess == qProcesses-1){
            new->lineas = cantidadLineas + (qLines%qProcesses);
        }
        pid = fork();
        if (pid > 0) { // padre
            close(pipes[numProcess][LECTURA]);
            write(pipes[numProcess][ESCRITURA], new, sizeof(mensaje)); // escribo en pipe  
            write(pipes[numProcess][ESCRITURA], nameFile, sizeof(char)*x);
            write(pipes[numProcess][ESCRITURA], chain, sizeof(char)*y);       
        }
        else{ //hijo
            close(pipes[numProcess][ESCRITURA]);
            dup2(pipes[numProcess][LECTURA], STDIN_FILENO); // copio lo que me envía el padre
            char* argumentos[4] = {"comparador", nameF, nameS, NULL};
            execv(argumentos[0], argumentos); // ejecuto otro programa
        }
        fseek(fp, cantidadLineas*count, SEEK_SET); // muevo el cursor a la siguiente linea que le correspondo al hijo siguiente
	}
    // lógica del padre
    // espero que terminen los hijos
    fclose(fp);
    for (numProcess = 0; numProcess < qProcesses; numProcess++){
		while ((pid=waitpid(-1,&status,0))!=-1){
		}
	}
    //libero la memoria del mensaje
    free(new);
    // libero memoria de los pipes
    for (numProcess = 0; numProcess < qProcesses; numProcess++){
        free(pipes[numProcess]);
    }
    free(pipes);
    // genero el nombre de archivo final
    char* salida = (char*)calloc((y+7),sizeof(char));
    strcat(salida, "rc_");
    strcat(salida, chain);
    strcat(salida, ".txt");
    FILE* fpout = fopen(salida, "w");
    // leo los resultados parciales de los archivos generados por los hijos
    int h = 0; // contador que almacenarara la cantidad de bytes del archivo final
    for (numProcess = 0; numProcess < qProcesses; numProcess++){
        // genero el nombre del archivo de salida del proceso correspondiente para leer  
        int n = sizeof(numProcess)/sizeof(int); // cantidad de digitos del identificador
        int qCaracteres = 8 + y + n; // cantidad de caracteres del archivo de salida 
        char *entrada = (char*)calloc(qCaracteres, sizeof(char)); // asigno memoria al string que tendrá el nombre del archivo de salida 
        char *number = (char*)calloc(n, sizeof(char)); // asigno memoria al string que tendrá el identificador del proceso
        sprintf(number, "%d", numProcess);  
        strcat(entrada, "rp_");
        strcat(entrada, chain);
        strcat(entrada, "_");
        strcat(entrada, number);
        strcat(entrada, ".txt");
        char* contenedor = (char*)calloc((count+6)*cantidadLineas, sizeof(char)); // almacenará la información del archivo que se lee
        FILE* fpin = fopen(entrada, "r");
        if (qLines % qProcesses != 0 && numProcess == qProcesses-1){ // si las lineas no se distribuyeron equitativamente entre los procesos , el archivo del último proceso tiene más lineas por lo que hay que dar más memoria para leer completamente el archivo 
            cantidadLineas = cantidadLineas + (qLines%qProcesses);
            contenedor = (char*)realloc(contenedor,(count+6)*cantidadLineas);
            fread(contenedor,sizeof(char),(count+6)*cantidadLineas,fpin);
            fwrite(contenedor,sizeof(char),(count+6)*cantidadLineas,fpout);
            h= h + (count+6)*cantidadLineas;
        }
        else{
            fread(contenedor,sizeof(char),(count+6)*cantidadLineas,fpin);
            fwrite(contenedor,sizeof(char),(count+6)*cantidadLineas,fpout);
            h = h + (count+6)*cantidadLineas;
        }
        free(entrada);
        free(number);
        free(contenedor);
        fclose(fpin);
	}
    fclose(fpout);
    // FLAGS 
    char* dContenedor = (char*)malloc(sizeof(char)*h);
    if (dflag == 1){
        FILE* fpout = fopen(salida, "r");
        fread(dContenedor,sizeof(char),h,fpout);
        printf("El resultado final es\n%s",dContenedor);
        fclose(fpout);
    }
    free(dContenedor);
    free(salida);
    return 0;
}