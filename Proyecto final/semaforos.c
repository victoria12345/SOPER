#include "semaforos.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>



/**
* @brief Inicializa un semaforo
*
* @param semid identificador del semaforo
* @param array array de los valores del semaforo
* @return OK si se realiza correctamente y ERROR si no ha sido asi
*
* @author Victoria Pelayo e Ignacio Rabunnal
*/
int Inicializar_Semaforo(int semid, unsigned short *array){
	union semun{
		int val;
		struct semid_ds *semstat;
		unsigned short *array;
	}arg;

	if(semid == -1 || array == NULL){
		return ERROR;
	}

	arg.array = array;
	
	if(semctl(semid,0,SETALL,arg) == -1){
		return ERROR;
	}

	return OK;

}

/**
* @brief Borra un semaforo
* @param semid Identificador del semaforo
* @return OK si se realiza correctamente y ERROR si no es asi
*
* @author Victoria Pelayo e Ignacio Rabunnal
*/

int Borrar_Semaforo(int semid){

	if(semctl(semid, 0, IPC_RMID) == -1){
		return ERROR;
	}

	return OK;
}

/**
* @brief Crea un semaforo
* @param key clave precompartida del semaforo
* @param size tamannio del semaforo
* @param semid identificador del semaforo
* @return 1 si estaba ya creado *semid si se ha creado y ERROR si no se ha podido crear
*
* @author Victoria Pelayo e Ignacio Rabunnal
*/
int Crear_Semaforo(key_t key, int size, int *semid){
	

	*semid = semget(key, size, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);

	if(*semid == -1 && errno == EEXIST){
		*semid = semget(key, size, SHM_R | SHM_W);
	}

	if(*semid == -1){
		return ERROR;
	}

	return OK;
/**
	arg.array = (unsigned short* )malloc(sizeof(unsigned short) * size);

	for(i = 0; i < size; i++){
		arg.array[i] = 0;
	}

	semctl(*(semid), size, SETALL, arg);
	free(arg.array);

	return 0;*/
}

/**
* @brief baja el semaforo indicado
*
* @param id identificador del semaforo
* @param num_sem semaforo dentro del array
* @param undo flag de modo persistente
* @return OK si se realiza correctamente y ERROR si no ha sido asi
*
* @author Victoria Pelayo e Ignacio Rabunnal
*/
int Down_Semaforo(int id, int num_sem, int undo){
	struct sembuf sem_oper;

	if(id == -1|| num_sem < 0 ){
		return ERROR;
	}

	sem_oper.sem_num = num_sem;
	sem_oper.sem_op = -1;
	sem_oper.sem_flg = undo;

	if(semop(id, &sem_oper, 1) == -1){
		printf("Despues (Down)%d %s\n", num_sem, strerror(errno));  
		return ERROR;
	}

	return OK;
}

/**
* @brief aumenta el semaforo indicado
*
* @param id identificador del semaforo
* @param num_sem semaforo dentro del array
* @param undo flag de modo persistente
* @return OK si se realiza correctamente y ERROR si no ha sido asi
*
* @author Victoria Pelayo e Ignacio Rabunnal
*/
int Up_Semaforo(int id, int num_sem, int undo){
	struct sembuf sem_oper;

	if(id == -1|| num_sem < 0 ){
		return ERROR;
	}

	sem_oper.sem_num = num_sem;
	sem_oper.sem_op = 1;
	sem_oper.sem_flg = undo;

	if(semop(id, &sem_oper, 1) == -1){
		printf("Despues del up ERROR%d %s\n", num_sem, strerror(errno));	
		return ERROR;
	}

	return OK;
}

/**
* @brief baja todos los semaforos del array indicado por active
*
* @param id identificador del semaforo
* @param size numero de semaforos
* @param undo flag de modo persistente pese a finalizacion abrupta
* @param active semaforos que se quieren bajar
* @return OK si se realiza correctamente y ERROR si no ha sido asi
*
* @author Victoria Pelayo e Ignacio Rabunnal
*/
int DownMultiple_Semaforo(int id,int size,int undo,int *active){
	int i;
	struct sembuf sem_oper[size];

	if(size < 1){
		return ERROR;
	}
	
	for(i = 0; i < size; i++){
		sem_oper[i].sem_num = active[i];
		sem_oper[i].sem_op = -1;
		sem_oper[i].sem_flg = undo;
	}

	if(semop(id, sem_oper, size) == -1){
		return ERROR;
	}

	return OK;
}

/**
* @brief sube todos los semaforos del array indicado por active
*
* @param id identificador del semaforo
* @param size numero de semaforos
* @param undo flag de modo persistente pese a finalizacion abrupta
* @param active semaforos que se quieren subir
* @return OK si se realiza correctamente y ERROR si no ha sido asi
*
* @author Victoria Pelayo e Ignacio Rabunnal
*/
int UpMultiple_Semaforo(int id,int size,int undo, int *active){
	int i;
	struct sembuf sem_oper[size];

	if(size < 1){
		return ERROR;
	}

	for(i = 0; i < size; i++){
		sem_oper[i].sem_num = active[i];
		sem_oper[i].sem_op = 1;
		sem_oper[i].sem_flg = undo;
	}

	if(semop(id, sem_oper, size) == -1){
		return ERROR;
	}

	return OK;
}
