#include "listas.h"

//Definir tipo

typedef struct Nodo {
    int valor;
    struct Nodo *prev;
    struct Nodo *next;
} Nodo;

//Funciones auxiliares

//Devuelve la dirección del último nodo de la lista
Nodo * finalDeLista(Lista *lista){
    Nodo *nodo = lista->raiz;

    while(nodo->next != NULL){
        nodo = nodo->next;
    }

    return nodo;
}

//*****IMPORTANTE***** no tiene protección ante valores que no esten en la lista *****IMPORTANTE*****
//Devuelve la dirección de un nodo con cierto valor
Nodo * buscarNodoValor(Nodo *nodo, int valor){
    while(nodo->valor != valor){
        nodo = nodo->next;
    }

    return nodo;
}

//Devuelve la dirección del nodo en la posicoion indicada o NULL
Nodo * buscarNodoPosicion(Nodo *nodo, int posicion){
    for(int i = 0; i < posicion && nodo != NULL; i++)
        nodo = nodo->next;

    //mensaje de error si el nodo es NULL
    if(nodo == NULL)
        perror("ERROR: LIST POSSITION OUT OF BOUNDS");

    return nodo;
}


//Funciones usables


//inicializa la lista
Lista * inicializar_lista(){
    Lista *lista;
    lista = malloc(sizeof(Lista));
    lista->raiz = NULL;

    return lista;
}

//Añade un valor al final de la lista
void addValorLista(Lista *lista, int valor){

    if(lista->raiz == NULL){
        //Caso especial en el que la lista está vacía

        //Asignar memoria
        lista->raiz = malloc(sizeof(struct Nodo));

        //Asignar valores y direcciones
        lista->raiz->valor = valor;
        lista->raiz->prev = NULL;
        lista->raiz->next = NULL;
    }
    else {
        //Caso genérico
        Nodo *Nodo = finalDeLista(lista);

        //Asignar memoria
        Nodo->next = malloc(sizeof(struct Nodo));

        //Asignar valores y direcciones
        Nodo->next->valor = valor;
        Nodo->next->prev = Nodo;
        Nodo->next->next = NULL;
    }
}

//Elimina un valor de la lista
void eliminarValorLista(Lista *lista, int valor){
    //Encontrar el valor
    Nodo *nodo = buscarNodoValor(lista->raiz, valor);

    if(nodo->prev == NULL){
        //Caso especial en el que el nodo sea la raíz

        //Reasignar direcciones
        nodo->next->prev = NULL;
        lista->raiz = nodo->next;

        //liberar memoria
        free(nodo);
    }else {
        //Caso genérico

        //Reasignar direcciones
        nodo->prev->next = nodo->next;
        nodo->next->prev = nodo->prev;

        //liberar memoria
        free(nodo);
    }
}

//Devuelve un 0 o 1 dependiendo de si el valor exise o no
int existeValorLista(Lista *lista, int valor){
    Nodo *nodo = lista->raiz;

    while(nodo != NULL){
        if(nodo->valor == valor)
            return 1;

        nodo = nodo->next;
    }

    return 0;
}

//Devuelve la posicion del valor en la lista [0,len-1] (-1 si no se encuentra)
int posicionValorLista(Lista *lista, int valor){
    Nodo  *nodo = lista->raiz;
    int cont = -1, check = 0;

    while(nodo != NULL && !check){
        if(nodo->valor == valor)
            check = 1;

        cont++;
        nodo = nodo->next;
    }

    if(check)
        return cont;
    else
        return -1;
}

//elimina una posicion de la lista
void eliminarPosicionLista(Lista *lista, int posicion){
    Nodo *nodo = buscarNodoPosicion(lista->raiz, posicion);
    //excepcion si el nodo es nulo
    if(nodo != NULL) {
        if (nodo->prev == NULL) {
            //Caso especial en el que el nodo sea la raíz

            //Reasignar direcciones
            nodo->next->prev = NULL;
            lista->raiz = nodo->next;

            //liberar memoria
            free(nodo);
        } else {
            //Caso genérico

            //Reasignar direcciones
            nodo->prev->next = nodo->next;
            nodo->next->prev = nodo->prev;

            //liberar memoria
            free(nodo);
        }
    }
}

void modificarPosicionLista(Lista *lista, int posicion, int valor){
    //crear nodo auxiliar
    Nodo *nodo = lista->raiz;

    //buscar la posicion
    nodo = buscarNodoPosicion(lista->raiz, posicion);

    //modificar el valor
    nodo->valor = valor;

}

int valorPosicionLista(Lista *lista, int posicon){
    //crear nodo auxiliar
    Nodo *nodo = lista->raiz;

    //buscar el nodo
    nodo = buscarNodoPosicion(lista->raiz, posicon);

    //devolver el valor
    return nodo->valor;
}

int longitudLista(Lista *lista){
    //definir variables
    Nodo *nodo = lista->raiz;
    int i = 0;

    //buscar fin de lista
    while(nodo != NULL){
        i++;
        nodo = nodo->next;
    }

    return i;
}

void eliminarLista(Lista *lista){
    //definir variables
    Nodo *nodo = lista->raiz, *aux;

    while(nodo != NULL){
        aux = nodo->next;
        free(nodo);
        nodo = aux;
    }

    free(lista);
}


//Cuenta cuantos valores que se dan existen en la lista
int contarValoresLista(Lista *lista, int valor){
    Nodo *nodo = lista->raiz;
    int cont = 0;


    while(nodo != NULL){
        if(nodo->valor == valor)
            cont++;

        nodo = nodo->next;
    }


    return cont;
}

//multiplica la sección de la lista dada [inicio, final)
void multiplicar_seccion_lista(Lista *lista, int inicio, int final, float factor){
    Nodo *nodo = buscarNodoPosicion(lista->raiz, inicio);

    for(int i = inicio; i < final; i++){
        nodo->valor *= factor;
        nodo = nodo->next;
    }


}