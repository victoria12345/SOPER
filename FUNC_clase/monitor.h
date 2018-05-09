#ifndef MONITOR_H
#define MONITOR_H

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
#include "apuestas.h"
#include "caballos.h"

#define TIEMPO 30

typedef struct _Compartida_monitor{
	int posicion[MAX_CABALLOS*2]; /*!< ultimas posiciones y  tiradas de los caballos*/
}Compartida_monitor;

void Iniciar_monitor(int semid, int n_caballos, Compartida * compartida);

void monitor_enproceso(int semid, int id_zone2, int *estado, int longitud, int n_caballos,Compartida_monitor* compartida, int n_apostadores);

#endif
