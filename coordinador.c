#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define LECTURA 0
#define ESCRITURA 1

typedef struct mensaje{
    int identificador;
    int lineas;
    int  posCursor;
}mensaje;

int main(int argc, char** argv){ // argc indica cantidad de argumentos y argv es un arreglo de caracteres que tiene los parametros de entrada por consola 
    char* nameFile = NULL;
    int qProcesses, qLines;
    char* chain = NULL;
    int dflag = 0;
    int c;
    opterr = 0;
    while ((c = getopt (argc, argv, "i:n:c:p:d")) != -1){ //getopt guarda los argumentos ingresados por consola y los guarda en c hasta que no quede ninguno
        switch (c)
        {
        case 'i':
            nameFile = optarg;
            break;

        case 'n':
            qProcesses = atoi(optarg); // convierto la entrada recibida como string a entero 
            break;

        case 'c':
            qLines = atoi(optarg);
            break;
        
        case 'p':
            chain = optarg;
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
    // obtengo longitud de la linea del archivo
    size_t len = 0;
    char* aux;
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
        // 0 para leer y 1 para escribir
        pipes[numProcess] = (int*)malloc(sizeof(int)*2);
        // inicializamos los pipes
        pipe(pipes[numProcess]);
    }
    
    // CREAR MULTIPLES HIJOS
    int status;
	pid_t pid;

    mensaje *new = (mensaje*)malloc(sizeof(mensaje));

    // parametros por argv
    int x = strlen(nameFile); // cantidad de carácteres del archivo de entrada
    int y = strlen(chain); // cantidad de carácteres de la cadena
    char nameF[x];
    sprintf(nameF, "%d", x); // guardo el num como string
    char nameS[y];
    sprintf(nameS, "%d", y);
	
    new->posCursor = 0;

	for (numProcess = 0; numProcess < qProcesses ; numProcess++){
        new->identificador = numProcess;
        new->lineas = cantidadLineas;
        new->posCursor = numProcess*(cantidadLineas*count);

        if (qLines % qProcesses != 0 && numProcess == qProcesses-1){
            new->lineas = cantidadLineas + (qLines%qProcesses);
        }

        mensaje *aviso = (mensaje*)malloc(sizeof(mensaje));
        pid = fork();
        
        if (pid > 0) {
            close(pipes[numProcess][LECTURA]);
            write(pipes[numProcess][ESCRITURA], new, sizeof(mensaje)); // escribo en pipe  
            write(pipes[numProcess][ESCRITURA], nameFile, sizeof(char)*x);
            write(pipes[numProcess][ESCRITURA], chain, sizeof(char)*y);       
        }
        
        else{
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

    // genero el nombre de archivo final
    char* salida = (char*)malloc(sizeof(char)*(y+7));
    strcat(salida, "rc_");
    strcat(salida, chain);
    strcat(salida, ".txt");
    FILE* fpout = fopen(salida, "w");
    int n = sizeof(qProcesses)/sizeof(int);
    printf("%s\n", salida);
    // leo los resultados parciales de los archivos generados por los hijos
    for (numProcess = 0; numProcess < qProcesses; numProcess++){
        // genero el nombre del archivo de salida del proceso correspondiente para leer  
        int n = sizeof(numProcess)/sizeof(int); // cantidad de digitos del identificador
        int qCaracteres = 8 + y + n; // cantidad de caracteres del archivo de salida 
        char *entrada = (char*)malloc(sizeof(char)*qCaracteres); // asigno memoria al string que tendrá el nombre del archivo de salida 
        char *number = (char*)malloc(sizeof(char)*n); // asigno memoria al string que tendrá el identificador del proceso
        sprintf(number, "%d", numProcess);  
        strcat(entrada, "rp_");
        strcat(entrada, chain);
        strcat(entrada, "_");
        strcat(entrada, number);
        strcat(entrada, ".txt");
        printf("%s\n", entrada);
        
        char* contenedor = (char*)malloc(sizeof(char)*(count+6)*cantidadLineas); // almacenará la información del archivo que se lee
        FILE* fpin = fopen(entrada, "r");
        if (qLines % qProcesses != 0 && numProcess == qProcesses-1){
            cantidadLineas = cantidadLineas + (qLines%qProcesses);
            contenedor = (char*)realloc(contenedor,(count+6)*cantidadLineas);
            fread(contenedor,sizeof(char),(count+6)*cantidadLineas,fpin);
            fwrite(contenedor,sizeof(char),(count+6)*cantidadLineas,fpout);
        }
        else{
            fread(contenedor,sizeof(char),(count+6)*cantidadLineas,fpin);
            fwrite(contenedor,sizeof(char),(count+6)*cantidadLineas,fpout);
        }
	}
    fclose(fpout);
    return 0;
}