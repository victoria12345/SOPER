#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <fcntl.h>
#include "semaforos.h"

typedef struct _Apuesta{
	long id;
	char apostador[30];
	int caballo;
	double apuesta;
}Apuesta;

typedef struct _Compartida{
	double *apuestas; /*!< Array con las apuestas*/
	double *cotizacion; /*!< Cotizaciones de los caballos*/
	int total_apuestas; /*!< total de las apuestas realizadas*/
}Compartida;

typedef struct _Estructura_hilo{
	int id;
	int id_zone;
	int id_mensajes;
	int n_caballos;
}Estructura_hilo;

void manejador_monitor(int sennal);

int Inicializar_apuestas(int n_caballos, Compartida* compartida);

void apostador(int id, int cantidad, int n_caballos, int id_mensajes);

int Inicializar_Gestor(Estructura_hilo* estructura_hilo,int semid,int n_sem,int n_caballos,int id_zone,int id_mensajes,Compartida* compartida);
