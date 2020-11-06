#ifndef MENSAJE_H
#define MENSAJE_H
typedef struct mensaje{
    char* secuencia;
    char* nombreArchivo;
    int identificador;
    int lineas;
    int  posCursor;
}mensaje;
#endif