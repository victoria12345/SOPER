#include "semaforos.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/shm.h>
#include <stdlib.h>



int Inicializar_semaforo(int semid, unsigned short *array){

	if(semid == -1 || array == NULL){
		return ERROR;
	}

	semctl(semid,0,SETALL,array);

	return OK;

}

int Borrar_Semaforo(int semid){

	return semctl(semid, 0, IPC_RMID);

}

int Crear_Semaforo(key_t key, int size, int *semid){
	*(semid) = semget(key, size, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);

	if(*(semid) == -1 && errno == EEXIST){
		*(semid) = semget(key, size, SHM_R | SHM_W);
		return 1;
	}

	if(*(semid == -1)){
		return ERROR;
	}

	union semun {
		int val;
		struct semid_ds *semstat;
		unsigned short *array;
	} arg;

	arg.array = (unsigned short* )malloc(sizeof(short) * size);

	for(int i = 0; i < size; i++){
		arg.array[i] = 0;
	}

	semctl(*(semid), size, SETALL, arg);

	return *(semid);
}

