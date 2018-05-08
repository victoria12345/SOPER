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

#define TIEMPO 20

typedef struct _Compartida_monitor{
	int posicion[MAX_CABALLOS]; /*!< posiciones de los caballos*/
	int tiradas[MAX_CABALLOS]; /*!< ultimas tiradas de los caballos*/
}Compartida_monitor;

void Iniciar_monitor(int semid, int n_caballos, Compartida * compartida);

#endif