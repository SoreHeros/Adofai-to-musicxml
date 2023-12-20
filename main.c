#include <stdio.h>
#include "listas.h"

//archivos
#define ARCHIVO_DATOS_ANGULOS "salida_datos_angulos.txt"
#define ARCHIVO_DATOS_TWIRLS "salida_datos_twirls.txt"
#define ARCHIVO_DATOS_VELOCIDADES "salida_datos_velocidades.txt"
#define ARCHIVO_DATOS_DELTA "salida_datos_delta.txt"
#define ARCHIVO_DATOS_VELOCIDADES_DIRECTAS "salida_datos_velocidades_directas.txt"
#define ARCHIVO_DATOS_VELOCIDADES_MULTIPLICATIVAS "salida_datos_velocidades_multiplicativas.txt"
#define ARCHIVO_DATOS_DURACIONES "salida_datos_duraciones.txt"
#define ARCHIVO_DATOS_DURACIONES_FORMATEADAS "salida_datos_duraciones_formateados.txt"
#define ARCHIVO_DATOS_VELOCIDADES_FINALES "salida_datos_velocidades_finales.txt"
#define ARCHIVO_DATOS_COMPASES "salida_datos_compases.txt"
#define ARCHIVO_RESULTADO "resultado.musicxml"

//valor al que se aproximan los multiplos, este valor debe ser una nota
#define DIVISION_MINIMA (15.0 / 4.0)


//avanza el punero del archivo hasta que encuentra el input, devuelve el return value para testear EOF
int look_in_file_for(FILE *f, char const input[]){
    //declarar variables
    char caracter;
    int i = 0, return_value;

    //Loop hasta que se encuentre el string
    do{
        //scan e imprimir lo que se lee
        return_value = fscanf(f, "%c", &caracter);
        //printf("%c", caracter);

        //avanza uno si se encuentra un caracter, si los encuentra todos saldrá del loop, si no reinicia el contador
        if(input[i] == caracter)
            i++;
        else
            i = 0;

    } while(input[i] != '\000' && return_value != EOF);

    return return_value;
}



void extraer_datos(FILE *f, char nombre_de_cancnion[], char nombre_de_artista[]) {
    //definir variables
    FILE *angulos, *twirls, *velocidades;
    float value;
    int aux;
    char caracter;


    //Encontrar angulos
    {
        //crear archivo
        angulos = fopen(ARCHIVO_DATOS_ANGULOS, "w");


        //buscar el principio del angledata ('[')
        look_in_file_for(f, "[");
        printf("Angledata: [");


        //guardar los ángulos hasta que se llegue al final (']')
        do {
            fscanf(f, "%f", &value);
            fscanf(f, "%c", &caracter);
            fprintf(angulos, "%10f\n", value);

            printf("%.0f%c ", value, caracter);
        } while (caracter != ']');

        fclose(angulos);
        printf("\n");
    }

    //encontrar el nombre de cancion y el artista
    look_in_file_for(f, "\"artist\": ");
    fscanf(f, "\"%s\"", nombre_de_artista);
    look_in_file_for(f, "\"song\": ");
    fscanf(f, "\"%s\"", nombre_de_cancnion);


    //Encontar velocidades y twirls
    {
        //abrir archivos
        velocidades = fopen(ARCHIVO_DATOS_VELOCIDADES, "w");
        twirls = fopen(ARCHIVO_DATOS_TWIRLS, "w");

        //Buscar el bpm inicial
        look_in_file_for(f, "\"bpm\":");
        printf("bpm: ");

        //guardar el bpm
        fscanf(f, "%f", &value);
        printf("%.0f\n", value);
        fprintf(velocidades, "%f\n", value);


        //loop de encontrar twirls y velocidades hasta EOF
        while (1) {


            //buscar inicio de parametro + condicion de salida
            if (look_in_file_for(f, "\"floor\":") == EOF)
                break;

            //guardar momentaneamente el suelo actual por si acaso se encuentra un valor adecuado
            fscanf(f, "%i", &aux);
            //printf(" %i", aux);

            //avanzar el puntero hasta una posición conveniente
            look_in_file_for(f, "\"eventType\": \"");

            fscanf(f, "%c", &caracter);
            //printf("%c", caracter);


            if (caracter == 'T') {
                //Si es T, significa que es un twirl el 100% de las veces, entonces lo añadimos
                fprintf(twirls, "%i\n", aux);
                printf("Floor: %i eventType: Twirl\n", aux);
            } else if (caracter == 'S') {
                //Si es S, hay dos opciones, que sea un SetSpeed (el que nos interesa) o un SetFilter
                look_in_file_for(f, "et");
                fscanf(f, "%c", &caracter);
                //printf("%c", caracter);

                if (caracter == 'S') {
                    //A este punto sabemos que estamos en un setspeed, ahora hace falta everiguar el tipo (Multiplier o directo)
                    look_in_file_for(f, "\"speedType\": \"");

                    fscanf(f, "%c", &caracter);
                    //printf("%c", caracter);
                    printf("Floor: %i eventType: SetSpeed speedType: ", aux);

                    if (caracter == 'M') {
                        //caso de que sea multiplicativo

                        //avanzar puntero
                        look_in_file_for(f, "\"bpmMultiplier\":");

                        //guardar valor
                        fscanf(f, "%f", &value);
                        //printf("%.2f", value);

                        //añadir al archivo
                        fprintf(velocidades, "%i 1 %f\n", aux, value);
                        printf("Multiplier: %f\n", value);
                    } else {
                        //caso de que sea directo

                        //avanzar puntero
                        look_in_file_for(f, "\"beatsPerMinute\":");

                        //guarvar valor
                        fscanf(f, "%f", &value);
                        //printf("%.2f", value);

                        //añadir al archivo
                        fprintf(velocidades, "%i 0 %f\n", aux, value);
                        printf("Direct: %f\n", value);
                    }
                }
            }
        }

        fclose(velocidades);
        fclose(twirls);
        fclose(f);
    }

}

float get_cercano(float val){
    int i;
    float aux;


    i = val / DIVISION_MINIMA;
    aux = val / DIVISION_MINIMA;

    if(aux - i <= 0.5 && aux - i >= -0.5)
        aux = i * DIVISION_MINIMA;
    else
        aux = (i+1) * DIVISION_MINIMA;


    return aux;
}

void hacer_delta(){
    //Hacer delta de los angulos

    //Abrir_archivos
    FILE *angulos = fopen(ARCHIVO_DATOS_ANGULOS, "r");
    FILE *twirls = fopen(ARCHIVO_DATOS_TWIRLS, "r");
    FILE *delta = fopen(ARCHIVO_DATOS_DELTA, "w");

    //definir variables
    int return_value, twirl_readout, twirl_state, next_twirl, cont, is_midspin;
    float angulo_act, angulo_prev, aux;

    //setup antes del loop
    //get proximos angulos
    fscanf(angulos, "%f", &angulo_prev);
    return_value = fscanf(angulos, "%f", &angulo_act);

    is_midspin = 0;

    angulo_act = get_cercano(angulo_act);
    angulo_prev = get_cercano(angulo_prev);

    //ver cuando es el proximo twirl
    twirl_readout = fscanf(twirls, "%i", &next_twirl);
    cont = 1;
    twirl_state = 0;


    //repetir loop hasta EOF
    while(return_value != EOF){
        //get delta
        aux = angulo_prev - angulo_act;

        //añadir el viaje extra si es que no es midspin
        if(!is_midspin)
            aux += 180;

        //normalizar el angulo para que esté entre (0 y 360], solo por si acaso
        while(aux>= 360)
            aux -= 360;
        while(aux<= 0)
            aux += 360;

        //actualizar el estado de twirl_state si es necesario
        if(cont >= next_twirl && twirl_readout != EOF) {
            twirl_state = !twirl_state;
            twirl_readout = fscanf(twirls, "%i", &next_twirl);
        }
        //Si pasa twirl_state hacer la inversa del angulo aux;
        if(twirl_state)
            aux = 360 - aux;

        //volver a normalizar el angulo
        while(aux>= 360)
            aux -= 360;
        while(aux<= 0)
            aux += 360;

        //guardar el dato obtenido
        fprintf(delta, "%f\n", aux);

        //obtener proximos datos
        angulo_prev = angulo_act;
        return_value = fscanf(angulos, "%f", &angulo_act);

        //comprobar si es midspin
        is_midspin = 0;
        if(angulo_act == 999.0){
            is_midspin = 1;
            fscanf(angulos, "%f", &angulo_act);
        }

        //convertir a multiplo de 15
        angulo_act = get_cercano(angulo_act);

        //actualizar contador
        cont++;

    }


    //cerrar archivos
    fclose(angulos);
    fclose(twirls);
    fclose(delta);

}

int es_factor_valido(float val){

    return val == 0.25 || val == 0.5 || val == 1 || val == 1.5 || val == 2 || val == 4;
}

//Normaliza las velocidades para que haya una (o varias, preferiblemente pocas) velocidades base y simplemente halla multiplicativas
void normalizar_velocidades(){
    //abrir archivos
    FILE *velocidades = fopen(ARCHIVO_DATOS_VELOCIDADES, "r");
    FILE *directas = fopen(ARCHIVO_DATOS_VELOCIDADES_DIRECTAS, "w");
    FILE *multiplicativas = fopen(ARCHIVO_DATOS_VELOCIDADES_MULTIPLICATIVAS, "w");

    //definir variables
    float vel_sec;
    float mul_act, factor;
    int ini_seccion, casilla_ini_seccion, casilla, modo;
    int file_readout;

    //iniciar listas
    Lista *casillas_directas = inicializar_lista();
    Lista *velocidades_directas = inicializar_lista();
    Lista *casillas_multiplicativas = inicializar_lista();
    Lista *velocidades_multiplicativas = inicializar_lista();

    //setup antes del bucle
    fscanf(velocidades, "%f", &vel_sec);
    mul_act = 1;
    ini_seccion = 0;
    casilla_ini_seccion = 0;

    addValorLista(casillas_multiplicativas, 0);
    addValorLista(velocidades_multiplicativas, 1 * 1000);

    while (1){
        //leer casilla
        file_readout = fscanf(velocidades, "%i", &casilla);
        if(file_readout == EOF)
            break;

        //leer modo
        fscanf(velocidades, "%i", &modo);

        //leer valor
        fscanf(velocidades, "%f", &factor);

        //compribar si es una asignación directa
        if(modo == 0) {
            //si lo es convertirla a un factor si es posible
            if (es_factor_valido(factor / vel_sec)) {
                factor /= vel_sec;
                modo = 1;
            }
        }


        //comprobar si es una asignación multiplicativa o directa y hacer los pasos convenientes
        if(modo){
            mul_act *= factor;

            //si el factor es menor a 1 multiplicar la sección de la lista y reasignar la velocidad base
            if(mul_act < 1) {
                multiplicar_seccion_lista(velocidades_multiplicativas, ini_seccion,longitudLista(velocidades_multiplicativas), 1 / mul_act);
                vel_sec *= mul_act;
                mul_act = 1;
            }

            //guardar el nuevo factor si no es igual a 1
            addValorLista(velocidades_multiplicativas, mul_act * 1000);
            addValorLista(casillas_multiplicativas, casilla);

        } else{
            //Estamos en una asignacion directa, por lo tanto, es necesario guardar la velocidad final de la sección e iniciar una nueva sección
            addValorLista(velocidades_directas, vel_sec * 1000);
            addValorLista(casillas_directas, casilla_ini_seccion);

            vel_sec = factor;
            casilla_ini_seccion = casilla;
            ini_seccion = longitudLista(velocidades_multiplicativas);

            //añadir un multiplicator x1 a la sección
            addValorLista(velocidades_multiplicativas, 1000);
            addValorLista(casillas_multiplicativas, casilla);
        }
    }

    //almacenar los datos de la última seccion
    addValorLista(velocidades_directas, vel_sec * 1000);
    addValorLista(casillas_directas, casilla_ini_seccion);


    //guardar los datos en los archivos
    printf("\n");
    for(int i = 0, aux = longitudLista(velocidades_multiplicativas); i < aux; i++){
        fprintf(multiplicativas, "%i %f\n", valorPosicionLista(casillas_multiplicativas, i), valorPosicionLista(velocidades_multiplicativas, i) / 1000.0);
        printf("%i %f\n", valorPosicionLista(casillas_multiplicativas, i), valorPosicionLista(velocidades_multiplicativas, i) / 1000.0);
    }

    printf("\n");
    for(int i = 0, aux = longitudLista(velocidades_directas); i < aux; i++){
        fprintf(directas, "%i %f\n", valorPosicionLista(casillas_directas, i), valorPosicionLista(velocidades_directas, i) / 1000.0);
        printf("%i %f\n", valorPosicionLista(casillas_directas, i), valorPosicionLista(velocidades_directas, i) / 1000.0);
    }


    //cerrar listas
    eliminarLista(casillas_directas);
    eliminarLista(velocidades_directas);
    eliminarLista(casillas_multiplicativas);
    eliminarLista(velocidades_multiplicativas);

    //cerrar archivos
    fclose(velocidades);
    fclose(directas);
    fclose(multiplicativas);
}


//devuelve cuantas divisiones hay
int divisiones(){
    float negra = 180;
    int nota_minima;

    //averiguar la duración del multiple más baja
    nota_minima = negra / DIVISION_MINIMA;

    //por cuestiones de formato es necesario multiplicar el resultado final por 3 si es que la division minima no son tresillos y por 2 si lo son
    if(nota_minima % 3)
        nota_minima *= 3;
    else
        nota_minima *= 2;

    return nota_minima;
}



void convertir_a_duraciones(){
    FILE *datos = fopen(ARCHIVO_DATOS_DELTA, "r");
    FILE *factores = fopen(ARCHIVO_DATOS_VELOCIDADES_MULTIPLICATIVAS, "r");
    FILE *salida = fopen(ARCHIVO_DATOS_DURACIONES, "w");
    int file_readout, factor_readout;
    float angle, divis, factor_act, factor_max = 1;
    int dur, cont, next_factor;

    //encontrar el factor máximo
    do {
        //dump el primer valor
        factor_readout = fscanf(factores, "%i", &next_factor);

        //get el factor
        if(factor_readout != EOF)
            fscanf(factores, "%f", &factor_act);

        //guardar el factor mayor
        if(factor_act > factor_max)
            factor_max = factor_act;

    } while (factor_readout != EOF);

    //imprimir cuantas divisiones hay, siendo multiplicadas por el factor más alto
    dur = (divisiones() * factor_max);
    divis = 180.0 / divisiones();
    fprintf(salida, "%i\n", dur);
    printf("%i\n", dur);

    //get el primer factor
    fseek(factores, 0, SEEK_SET);
    fscanf(factores, "%i %f", &next_factor, &factor_act);
    factor_readout = fscanf(factores, "%i", &next_factor);
    cont = 1;

    //repetir hasta EOF
    file_readout = fscanf(datos, "%f", &angle);
    while (file_readout != EOF){
        //get el proximo factor si se ha alcanzado si posicion
        if(cont >= next_factor && factor_readout != EOF) {
            //si se alcanza EOF next_factor será 0, por lo que nunca será igual a cont
            fscanf(factores, "%f", &factor_act);
            factor_readout = fscanf(factores, "%i", &next_factor);
        }




        dur = ((angle * factor_max) / (divis  * factor_act));
        fprintf(salida, "%3i ", dur);
        printf("%i, ", dur);
        file_readout = fscanf(datos, "%f", &angle);


        //cont es la casilla actual
        cont++;
    }


    //cerrar archivos
    fclose(salida);
    fclose(factores);
    fclose(datos);
}

//reformatea las duraciones a otro formato más útil
void reformatear_datos(int compas, int factor_manual){
    //abrir archivos
    FILE *duraciones = fopen(ARCHIVO_DATOS_DURACIONES, "r");
    FILE *duraciones_formateadas = fopen(ARCHIVO_DATOS_DURACIONES_FORMATEADAS, "w");
    FILE *velocidades = fopen(ARCHIVO_DATOS_VELOCIDADES_DIRECTAS, "r");
    FILE *velocidades_formateadas = fopen(ARCHIVO_DATOS_VELOCIDADES_FINALES, "w");

    //inicializar lista
    Lista *lista = inicializar_lista();

    //get divisiones
    int divisiones, file_readout, aux, dato, max, min, factor_maximo, factor_generico;
    fscanf(duraciones, "%i", &divisiones);
    fscanf(duraciones, "%i", &dato);
    max = min = dato;

    do{
        //añadir valores a la lista
        addValorLista(lista, dato);

        //get valor maximo
        if(dato > max)
            max = dato;

        //get valor minimo
        if(dato < min)
            min = dato;

        //get siguiente valor
        file_readout = fscanf(duraciones, "%i", &dato);
    } while (file_readout != EOF);


    //buscar la nuva división menor con el valor minimo
    //convertir la división a "su 1"
    if(min % 3)
        min /= 2;
    else
        min /= 3;


    for(int i = 0, max = longitudLista(lista); i < max; i++){
        dato = valorPosicionLista(lista, i);
        if(dato % min)
            min = dato % min;
    }

    //reasignarle el valor a min
    min *= 2;


    //get factor de los datos
    factor_maximo = 1;
    while(max <= divisiones * compas){
        factor_maximo *= 2;
        max *= 2;
    }
    //dividir entre 2 por la sobre compensación
    factor_maximo /= 2;

    //get factor de divisiones
    factor_generico = 1;
    while(min >= 2){
        factor_generico *= 2;
        min /= 2;
    }
    //dividir entre 2 para evitar sobrecompensar
    factor_generico /= 2;

    //get nuevas divisiones
    divisiones = (divisiones * factor_manual) / (factor_generico * factor_maximo);

    //imprimir el nuevo factor de divisiones
    fprintf(duraciones_formateadas, "%i\n", divisiones);
    printf("\n\n\n\n%i\n", divisiones);
    //imprimir el resto de datos
    for(int i = 0, max = longitudLista(lista); i < max; i++){
        aux = (valorPosicionLista(lista, i)) / (factor_generico);
        fprintf(duraciones_formateadas, "%3i ", aux);
        printf("%i ", aux);
    }
    printf("\n\n\n");

    //reformatear velocidades
    float velocidad;
    int casilla;
    fscanf(velocidades, "%i %f", &casilla, &velocidad);
    do{
        fprintf(velocidades_formateadas, "%i %f\n", casilla, velocidad * factor_maximo / factor_manual);
        printf("%i %f\n", casilla, velocidad * factor_maximo / factor_manual);
        file_readout = fscanf(velocidades, "%i %f", &casilla, &velocidad);
    }while(file_readout != EOF);





    //cerrar archivos
    fclose(duraciones);
    fclose(duraciones_formateadas);
    fclose(velocidades);
    fclose(velocidades_formateadas);
}

//devuelve la jota (pura) mas cercana o el tresillo, si es puro
int get_nota_cercana(int valor){
    int nota_cercana = 1;
    if(nota_cercana % 3)
        while (nota_cercana <= valor){
            if(nota_cercana == valor)
                return nota_cercana;
            nota_cercana *= 2;
        }

    nota_cercana = 3;
    while (nota_cercana <= valor)
        nota_cercana *= 2;

    return nota_cercana / 2;
}

//los numeros negativos son silencios, los compases están separados por | al final de cada uno
void convertir_a_compases(int compas){
    //abrir archivos
    FILE *duraciones = fopen(ARCHIVO_DATOS_DURACIONES_FORMATEADAS, "r");
    FILE *compases = fopen(ARCHIVO_DATOS_COMPASES, "w");

    int duracion_del_compas, duracion_negra, duracion_actual, nota, aux, aux2, file_readout, contador = 1;

    //get duracion_del_compas e imprimir las divisiones al principio del archivo
    fscanf(duraciones, "%i", &duracion_negra);
    fprintf(compases, "%i\n", duracion_negra);
    printf("\n\n\n%i\n", duracion_negra);
    duracion_del_compas = compas * duracion_negra;
    char tipo_de_cabeza, puntillo, tresillo;


    //inicialización antes del bucle
    duracion_actual = 0;
    fscanf(duraciones, "%i", &nota);
    do{
        //asignarle a la nota lo máximo que quepa en este compás si se sobrepasa
        duracion_actual += nota;
        if(duracion_actual > duracion_del_compas)
            nota -= duracion_actual - duracion_del_compas;


        //Asignación temporal del tipo de nota como x todo mientras no se implementen notas sostenidas
        tipo_de_cabeza = 'x';
        tresillo = '_';
        while (nota > 0){
            puntillo = '_';
            aux = get_nota_cercana(nota);
            nota -= aux;

            //aux2 es para ver si es un puntillo
            aux2 = get_nota_cercana(nota);
            if(aux == aux2 * 2) {
                puntillo = '.';
                nota -= aux2;
            }

            //comprobar si es un tresillo y darle su valor adecuado
            if(aux % 3){
                aux = aux * 3 / 2;
                tresillo = 't';
            }

            fprintf(compases, " %c%c%c%i", tipo_de_cabeza, puntillo, tresillo, aux);

            //a partir de la primera nota escribir silencios
            tipo_de_cabeza = 's';
        }

        //escribir los silenciones al principio del siguiente compás si se sobresale
        if(duracion_actual >= duracion_del_compas){
            //escribir un enter al final del compas
            contador++;
            if(contador == 16)
                printf("test");

            fprintf(compases, "\n");

            //si hay algo de duración residual escribir los silencios corespondientes
            duracion_actual -= duracion_del_compas;
            nota = duracion_actual;

            //asignarle a la cabeza el silencio por si acaso
            tipo_de_cabeza = 's';
            while (nota > 0){
                puntillo = '_';
                aux = get_nota_cercana(nota);
                nota -= aux;

                //aux2 es para ver si es un puntillo
                aux2 = get_nota_cercana(nota);
                if(aux == aux2 * 2) {
                    puntillo = '.';
                    nota -= aux2;
                }

                //comprobar si es un tresillo y darle su valor adecuado
                if(aux % 3){
                    aux *= 3;
                    tresillo = 't';
                }

                if(aux % 3)
                    aux *= 3;
                fprintf(compases, " %c%c%c%i", tipo_de_cabeza, puntillo, tresillo, aux);


            }
        }

        file_readout = fscanf(duraciones, "%i", &nota);
    }while(file_readout != EOF);



    //cerrar archivos
    fclose(duraciones);
    fclose(compases);
}

//recibe un valor (inverso) y devuelve el nombre de la nota
void tipo_nota(char nota[10],int valor){

    switch (valor) {
        case 1:
            sprintf(nota, "whole");
            break;
        case 2:
            sprintf(nota, "half");
            break;
        case 4:
            sprintf(nota, "quarter");
            break;
        case 8:
            sprintf(nota, "eighth");
            break;
        case 32:
            sprintf(nota, "32nd");
            break;
        default:
            if(valor % 10 == 2)
                sprintf(nota, "%ind", valor);
            else
                sprintf(nota, "%ith", valor);
            break;
    }
}


//el duracion es la inversa
void escribir_notas(FILE *archivo, char tipo_de_cabeza, char puntillo, char tresillo, int duracion, int duracion_compas){
    char nombre_nota[10];
    int aux;





    //get nueva duracion y aux


    //get nombre de nota
    tipo_nota(nombre_nota, duracion_compas / duracion);

    //get duración verdadera si es que es un tresillo
    if(tresillo == 't')
        duracion = duracion * 2 / 3;

    //imprimir el principio de la nota
    if(tipo_de_cabeza == 's')
        fprintf(archivo, "\n"
                         "      <note>\n"
                         "        <rest/>");
    else
        fprintf(archivo, "\n"
                         "      <note>\n"
                         "        <unpitched>\n"
                         "          <display-step>E</display-step>\n"
                         "          <display-octave>4</display-octave>\n"
                         "          </unpitched>");

    //imprimir duracion
    fprintf(archivo, "\n"
                     "        <duration>%i</duration>", duracion);

    //imprimir siguiente parte
    fprintf(archivo,"\n"
                    "        <voice>1</voice>");

    //imprimir tipo de nota
    fprintf(archivo, "\n"
                     "        <type>%s</type>", nombre_nota);

    //imprimir el puntillo
    if(puntillo == '.')
        fprintf(archivo, "\n"
                         "        <dot/>");

    //imprimir el tresillo si es tresillo
    if(tresillo == 't')
            fprintf(archivo, "\n"
                             "        <time-modification>\n"
                             "          <actual-notes>3</actual-notes>\n"
                             "          <normal-notes>2</normal-notes>\n"
                             "          </time-modification>");

    //imprimir siguiente parte
    if(tipo_de_cabeza == 'x')
        fprintf(archivo, "\n"
                         "        <notehead>x</notehead>");

    //imprimir parte final
    fprintf(archivo, "\n"
                     "        </note>");
}


void escribir_archivo_final(int compas, char nombre_de_cancion[], char nombre_de_artista[]){
    //abrir archivos
    FILE *resultados = fopen(ARCHIVO_RESULTADO, "w");
    FILE *compases = fopen(ARCHIVO_DATOS_COMPASES, "r");
    FILE *velocidades = fopen(ARCHIVO_DATOS_VELOCIDADES_FINALES, "r");

    //variables
    int casilla_velocidad, duracion, compas_restante, aux, duracion_compas, file_readout, compas_actual, contador_casilla, cambio_velocidad, velocidad_readout;
    float velocidad;
    char tipo_de_cabeza, puntillo, tresillo;

    //escribir el principio del archivo
    fprintf(resultados, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                    "<!DOCTYPE score-partwise PUBLIC \"-//Recordare//DTD MusicXML 4.0 Partwise//EN\" \"http://www.musicxml.org/dtds/partwise.dtd\">\n"
                                    "<score-partwise version=\"4.0\">\n"
                                    "  <work>\n"
                                    "    <work-title>%s</work-title>\n"
                                    "    </work>\n"
                                    "  <identification>\n"
                                    "    <creator type=\"composer\">%s</creator>\n"
                                    "    <encoding>\n"
                                    "      <software>MuseScore 4.1.1</software>\n"
                                    "      <encoding-date>2023-11-08</encoding-date>\n"
                                    "      <supports element=\"accidental\" type=\"yes\"/>\n"
                                    "      <supports element=\"beam\" type=\"yes\"/>\n"
                                    "      <supports element=\"print\" attribute=\"new-page\" type=\"no\"/>\n"
                                    "      <supports element=\"print\" attribute=\"new-system\" type=\"no\"/>\n"
                                    "      <supports element=\"stem\" type=\"yes\"/>\n"
                                    "      </encoding>\n"
                                    "    </identification>\n"
                                    "  <part-list>\n"
                                    "    <score-part id=\"P1\">\n"
                                    "      <part-name>Caja clara</part-name>\n"
                                    "      <part-abbreviation>Caj. Cla. </part-abbreviation>\n"
                                    "      <score-instrument id=\"P1-I38\">\n"
                                    "        <instrument-name>Side Stick</instrument-name>\n"
                                    "        </score-instrument>\n"
                                    "      <score-instrument id=\"P1-I39\">\n"
                                    "        <instrument-name>Snare</instrument-name>\n"
                                    "        </score-instrument>\n"
                                    "      <midi-device port=\"1\"></midi-device>\n"
                                    "      <midi-instrument id=\"P1-I38\">\n"
                                    "        <midi-channel>10</midi-channel>\n"
                                    "        <midi-program>49</midi-program>\n"
                                    "        <midi-unpitched>38</midi-unpitched>\n"
                                    "        <volume>78.7402</volume>\n"
                                    "        <pan>0</pan>\n"
                                    "        </midi-instrument>\n"
                                    "      <midi-instrument id=\"P1-I39\">\n"
                                    "        <midi-channel>10</midi-channel>\n"
                                    "        <midi-program>49</midi-program>\n"
                                    "        <midi-unpitched>39</midi-unpitched>\n"
                                    "        <volume>78.7402</volume>\n"
                                    "        <pan>0</pan>\n"
                                    "        </midi-instrument>\n"
                                    "      </score-part>\n"
                                    "    </part-list>\n"
                                    "  <part id=\"P1\">\n"
                                    "    <measure number=\"1\">\n"
                                    "      <attributes>", nombre_de_cancion, nombre_de_artista);

    //get divisiones
    fscanf(compases, "%i", &duracion_compas);

    //escribir divisiones
    fprintf(resultados ,"\n        <divisions>%i</divisions>", duracion_compas);
    duracion_compas *= compas;

    //escribir lo próximo
    fprintf(resultados,   "\n"
                                      "        <key>\n"
                                      "          <fifths>0</fifths>\n"
                                      "          </key>\n"
                                      "        <time>\n"
                                      "          <beats>4</beats>\n"
                                      "          <beat-type>4</beat-type>\n"
                                      "          </time>\n"
                                      "        <clef>\n"
                                      "          <sign>percussion</sign>\n"
                                      "          <line>2</line>\n"
                                      "          </clef>\n"
                                      "        <staff-details>\n"
                                      "          <staff-lines>1</staff-lines>\n"
                                      "          </staff-details>\n"
                                      "        </attributes>\n"
                                      "      <direction placement=\"above\">\n"
                                      "        <direction-type>\n"
                                      "          <metronome parentheses=\"no\">\n"
                                      "            <beat-unit>quarter</beat-unit>\n"
                                      "            <per-minute>");

    //get velocidad base
    fscanf(velocidades, "%i %f", &casilla_velocidad, &velocidad);

    //escribir velocidad
    fprintf(resultados, "%f", velocidad);

    //escribir siguiente parte
    fprintf(resultados, "</per-minute>\n"
                                    "            </metronome>\n"
                                    "          </direction-type>\n"
                                    "        <sound tempo=\"");

    //escribir velocidad otra vez
    fprintf(resultados, "%f", velocidad);

    //escribir siguiente parte del texto
    fprintf(resultados, "\"/>\n"
                                    "        </direction>");


    //ya se ha escrito el setup del principio, ahora quedan escribir las otras notas
    compas_actual = 1;
    compas_restante = duracion_compas;
    fscanf(compases, " %c%c%c%i", &tipo_de_cabeza, &puntillo, &tresillo, &duracion);
    contador_casilla = 0;
    velocidad_readout = fscanf(velocidades, "%i", &cambio_velocidad);
    do{
        if(tipo_de_cabeza != 's')
            contador_casilla++;

        if(contador_casilla >= cambio_velocidad && velocidad_readout != EOF){
            fscanf(velocidades, "%f", &velocidad);
            fprintf(resultados, "<direction placement=\"above\">\n"
                                "        <direction-type>\n"
                                "          <metronome parentheses=\"no\">\n"
                                "            <beat-unit>quarter</beat-unit>\n"
                                "            <per-minute>%f</per-minute>\n"
                                "            </metronome>\n"
                                "          </direction-type>\n"
                                "        <sound tempo=\"%f\"/>\n"
                                "        </direction>", velocidad, velocidad);
            velocidad_readout = fscanf(velocidades, "%i", &cambio_velocidad);
        }

        //escribir las notas correspondientes a la duración
        escribir_notas(resultados, tipo_de_cabeza, puntillo, tresillo, duracion, duracion_compas);

        //ver si se ha llegado al final del compas
        aux = duracion;
        if(puntillo == '.')
            aux = aux * 3 / 2;

        if(tresillo == 't')
            aux = aux * 2 / 3;

        compas_restante -= aux;
        if(compas_restante <= 0){
            //se ha llegado al final del compas, aumentar el numero de compas y escribir lo que sea necesario
            fprintf(resultados, "\n</measure>\n"
                                            "    <measure number=\"");

            //imprimir el numero de comás
            compas_actual++;
            fprintf(resultados, "%i", compas_actual);

            //escribir final de nuevo compas
            fprintf(resultados, "\">");


            //reiniciar el compas restante
            compas_restante = duracion_compas;
        }


        //get proximo valor
        file_readout = fscanf(compases, " %c%c%c%i", &tipo_de_cabeza, &puntillo, &tresillo, &duracion);



    } while (file_readout != EOF);

    //terminar de escribir el último compás si es necesario
    tipo_de_cabeza = 'x';
    while (compas_restante > 0){
        puntillo = '_';
        tresillo = '_';
        aux = get_nota_cercana(compas_restante);
        compas_restante -= aux;

        //comprobar si es un tresillo y darle su valor adecuado
        if(aux % 3){
            aux = aux * 3 / 2;
            tresillo = 't';
        }



        escribir_notas(resultados, tipo_de_cabeza, puntillo, tresillo, aux, duracion_compas);
        tipo_de_cabeza = 's';
    }




    //escribir el final del texto
    fprintf(resultados, "<barline location=\"right\">\n"
                                    "        <bar-style>light-heavy</bar-style>\n"
                                    "        </barline>\n"
                                    "      </measure>\n"
                                    "    </part>\n"
                                    "  </score-partwise>");

    //cerrar archivos
    fclose(resultados);
    fclose(compases);
    fclose(velocidades);
}



int main(){

    FILE *f;

    do {
        //este es el archivo por defecto
        char archivo[100] = "final_phase.adofai";
        char nombre_de_cancion[200], nombre_de_artista[200];
        printf("Introduce el nombre del archivo a parsear:\n");
        //scanf("%99s", archivo);

        //pedir los factores manuales, la anacrusa está en negras
        //TODO IMPLEMENTAR PARTES LENTAS, implementar anacrusa
        int factor_manual = 4, anacrusa = 2, compas = 4;
        printf("Introduce los factores manuales (anacrusa, compas, factor manual):\n");
        //scanf("%i %i %i", &anacrusa, &compas, &factor_manual);

        f = fopen(archivo, "r");

        if (f == NULL) {
            perror("ERROR: ARCHIVO NO ENCONTRADO");
        } else {
            //Extrae los datos útiles del archivo y lo cierra
            extraer_datos(f, nombre_de_cancion, nombre_de_artista);

            //A partir de los datos extraidos saca la diferencia entre ellos, hace los procesos necesarios para incluir los twirls
            hacer_delta();

            //normaliza las velocidades
            normalizar_velocidades();

            //Utiliza los datos normalizados para sacar la duración en un formato sencillo utilizando la velocidad multiplicativa
            convertir_a_duraciones();

            //minimiza el valor de las divisiones y los valores, también hace que el valor de duración más grande sea como máximo COMPAS veces las divisiones, y reajusta las velocidades directas
            reformatear_datos(compas, factor_manual);

            //Convierte las duraciones al formato para escribir, con puntillos, tresillos y lo necesario
            convertir_a_compases(compas);

            sprintf(nombre_de_cancion, "nombre cancion");
            sprintf(nombre_de_artista, "nombre artista");

            //hace al archivo musicxxml
            escribir_archivo_final(compas, nombre_de_cancion, nombre_de_artista);
        }

    } while (f == NULL);



    return 0;
}
