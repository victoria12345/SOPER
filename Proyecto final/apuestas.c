#include "apuestas.h"

int Inicializar_apuestas(int n_caballos, Compartida* compartida){
	int i;

	compartida->total_apuestas = n_caballos;

	compartida->apuestas = (double*)malloc(sizeof(double)*n_caballos);
	compartida->cotizacion = (double*)malloc(sizeof(double)*n_caballos);

	if(compartida->apuestas == NULL || compartida->cotizacion == NULL){
		return -1;
	}

	for(i = 0; i < n_caballos;i++){
		compartida->apuestas[i] = 1.0;
		compartida->cotizacion[i] = 1.0 / compartida->total_apuestas;
	}

	return 0;
}

void* ventanilla(void* estructura_hilo);