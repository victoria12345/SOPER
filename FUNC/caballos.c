#include "caballos.h"

/**
* Se activa cuando el caballo tiene que seguir corriendo
* Simplemente el caballo sigue con la carrera
* @param senna sennal que recibe
*/
void manejador_carrera(int sennal){
}

/**
* Se activa cuando el caballo tiene que terminal la carrera
* @param sennal sennal que recibe
*/
void manejador_final(int sennal){
	exit(EXIT_SUCCESS);
}

/**
* Funcion que simula el comportamiento del caballo
*/
int caballo(int tipo_dado, int id_mensajes, int pos){
	Mensaje msj;

	if(id_mensajes < 0 || pos < 0){
		return -1;
	}

	if(tipo_dado == DADO_NORMAL){
		msj.tirada = rand() %6 +1;
	}else if(tipo_dado == DOBLE_DADO){
		msj.tirada = (rand() %6 +1) + (rand() % 6 +1);
	}else{
		msj.tirada = rand() %7 +1;
	}

	msj.id = pos + 2;
	msgsnd(id_mensajes, (struct msgbuf*)&msj, sizeof(Mensaje)-sizeof(long), IPC_NOWAIT);

	return 0;
}

/**
* Calcula la posicion del caballo para saber que tirada le toca
* @param array array de las posiciiones de los caballos
* @param pos posicion del caballo que interesa
* @param longitud longitud del array
*
* @return 1 tirada normal, 0 si es dado del 1 al 7, 2 si es dado remontador
*/
int calcular_tirada(int* array, int pos, int longitud){
	int i;
	int primero = array[0];
	int ultimo = array[0];

	if(array == NULL || pos < 0 || pos > longitud || longitud < 0){
		return -1;
	}

	for(i = 0; i < longitud; i++){
		if(array[i] > primero){
			primero = array[i];
		}
		if(array[i] < ultimo){
			ultimo = array[i];
		}
	}
	/*Dado especial de 7*/
	if(primero == array[pos]){
		return SIETE_CARAS;
	}
	/*dos dados normales*/
	if(ultimo == array[pos]){
		return DOBLE_DADO;
	}
	/*Dado normal*/
	return DADO_NORMAL;
}