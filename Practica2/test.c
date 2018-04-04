#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/shm.h>
#include <stdlib.h>
#include "semaforos.h"

#define SEMKEY 75798

int main(void){

	union semun {
		int val;
		struct semid_ds *semstat;
		unsigned short *array;
	} arg;
	unsigned short* array =(unsigned short* )malloc(sizeof(unsigned short)*4);

	arg.array = (unsigned short* )malloc(sizeof(unsigned short) * 3);
	arg.array[0] = 0;
	arg.array[1] = 0;
	arg.array[2] = 0;



	int sem_id; /* ID de la lista de semáforos */

	if(Crear_Semaforo(SEMKEY, 3, &sem_id) == 0){
		printf("Prueba 1 Crear Semaforo superada\n");
	}else{
		printf("Prueba 1 Crear Semaforo no superada\n");
	}

	if(Crear_Semaforo(SEMKEY, 3, &sem_id) == 1){
		printf("Prueba 2 Crear Semaforo superada\n");
	}else{
		printf("Prueba 2 Crear Semaforo no superada\n");
	}

	semctl (sem_id, 3, SETALL, arg);

	if(Inicializar_semaforo(sem_id, array) == OK){
		printf("Prueba 1 inicializar Semaforo superada\n");
	}else{
		printf("Prueba 1 inicializar Semaforo no superada\n");
	}

	if(Up_Semaforo(sem_id, 1, SEM_UNDO) == OK){
		printf("Prueba 1 up Semaforo superada\n");
	}else{
		printf("Prueba 1 up Semaforo no superada\n");
	}

	if(Down_Semaforo(sem_id, 1, SEM_UNDO) == OK){
		printf("Prueba 1 down Semaforo superada\n");
	}else{
		printf("Prueba 1 down Semaforo no superada\n");
	}


	if(Borrar_Semaforo(sem_id) == 1){
		printf("Prueba 1 borrar Semaforo superada\n");
	}else{
		printf("Prueba 1 borrar Semaforo no superada\n");
	}


	return 0;
}
