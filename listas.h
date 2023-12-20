#ifndef ABB_LIBRARY_H
#define ABB_LIBRARY_H

#include <stdio.h>
#include <malloc.h>

//Tipos de datos

//Definir el tipo lista
typedef struct Lista {
    struct Nodo *raiz;
} Lista;

//Funciones

Lista * inicializar_lista();

void addValorLista(Lista *lista, int valor);

void eliminarValorLista(Lista *lista, int valor);

int existeValorLista(Lista *lista, int valor);

int posicionValorLista(Lista *lista, int valor);

void eliminarPosicionLista(Lista *lista, int posicion);

void modificarPosicionLista(Lista *lista, int posicion, int valor);

int valorPosicionLista(Lista *lista, int posicion);

int longitudLista(Lista *lista);

void eliminarLista(Lista *lista);

int contarValoresLista(Lista *lista, int valor);

void multiplicar_seccion_lista(Lista *lsita, int inicio, int final, float factor);






#endif