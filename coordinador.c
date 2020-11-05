#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
typedef struct mensaje{
    char* secuencia; // listo
    char* nombreArchivo; //listo
    int identificador;   //listo
    int lineas; // falta
    int  posCursor;  // listo
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
    int numProcess;
    // CREAR MULTIPLES PIPES
    // creamos una matriz donde cada fila será un pipe de un proceso hijo 
    int** pipes = (int**)malloc(sizeof(int*)*qProcesses);
    for (numProcess = 0; numProcess < qProcesses; numProcess++){
        // 0 para leer y 1 para escribir
        pipes[numProcess] = (int*)malloc(sizeof(int)*2);
    }
    // inicializamos los pipes
    for (numProcess = 0; numProcess < qProcesses; numProcess++){
        pipe(pipes[numProcess]);
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
    // AHORA A CREAR LOS PROCESOS HIJOS
    int status;
	pid_t pid; // arreglo de pids
	mensaje *new = (mensaje*)malloc(sizeof(mensaje));
	new->posCursor = 0;  
	for (numProcess = 0; numProcess < qProcesses ; numProcess++) {
        // le doy los argumentos al proceso hijo por el pipe  
        new->secuencia = chain;
        new->nombreArchivo = nameFile;
        new->identificador = numProcess;
        new->lineas = cantidadLineas;
        new->posCursor = numProcess*(cantidadLineas*count);
        // si las lineas no se repiten equitativamente, el último hijo las lee 
        if (qLines % qProcesses != 0 && numProcess == qProcesses-1){
            new->lineas = cantidadLineas + (qLines%qProcesses);
        }  
        write(pipes[numProcess][1], new, sizeof(mensaje)); // escribo en pipe


        pid = fork();
        if (pid == 0) {		// padre crea a siguiente hijo 		
			break;         
        }
        // el padre no lee así que cerramos la entrada de lectura de los pipe creados
        close(pipes[numProcess][0]);
        fseek(fp, cantidadLineas*count, SEEK_SET); // muevo el cursor a la siguiente linea que le correspondo al hijo siguiente
	}
    if (pid == 0) { //arreglar
       // Lógica del Hijo 
       // el hijo lee por lo tanto cerramos la entrada de escritura , además también cerramos las copias de los pipe de los otros hijos
       for (int i = 0; i < qProcesses; i++){
           for (int j = 0; j < 2; j++){
               if ( i != new->identificador){ // si no es el pipe del proceso hijo actual entonces cerramos la entrada de escritura y lectura  
                   close(pipes[i][j]); 
               }
               else{
                   if (j == 1){ // cierro la escritura del hijo actual 
                       close(pipes[i][j]);
                   }      
               }
           }
       }
       dup2(pipes[new->identificador][0], STDIN_FILENO); // copio lo que me envía el padre
       char* argumentos[2] = {"comparador", NULL};
	   printf("Wa ha ha soy el hijo: %d\n",new->identificador);
	   printf("la cadena que tengo que leer es: %s\n", new->secuencia);
	   printf("la cantidad de lineas que tengo que leer es: %d\n",new->lineas);
	   printf("el nombre del archivo que tengo que leer es: %s\n", new->nombreArchivo);
	   printf("tengo que leer desde la posicion %d del cursor\n", new->posCursor);
       execv(argumentos[0], argumentos); // ejecuto otro programa  --- no me lo tomaaaaa!!!!!!
	}

	else{
        // lógica del padre
        // espero que terminen los hijos
        for (numProcess = 0; numProcess < qProcesses; numProcess++){
			while ((pid=waitpid(-1,&status,0))!=-1){
				//printf("Process %d terminated\n",pid);
			}
		}
    }
    return 0;
}