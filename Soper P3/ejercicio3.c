#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <errno.h>
#include <sys/shm.h> 
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "semaforos.h"
#define FILEKEY "/bin/cat"
#define KEY 2345
#define TAM_ABC 27
#define NUMS 10

#define SEMKEY 12345


struct info{
	int nums[NUMS];
	char letras[TAM_ABC];
};


int main(int argc, char const *argv[]){
	int i;
  	int key;
  	int id_zone; 
  	int pid;
  	int semid;
  	int nsem = 1;

  	int *status = NULL;

  	unsigned short* semaforos;
  	struct info* produccion;


	if(Crear_Semaforo(SEMKEY,nsem, &semid) == ERROR){
    	printf("Error creando el primer semaforo\n");
    	exit(EXIT_FAILURE);
  	} 

  	if( Inicializar_Semaforo(semid, semaforos) == ERROR){
		Borrar_Semaforo(semid);
		printf("Error al inicializar el semaforo1\n");
		exit(EXIT_FAILURE);
	}

	key = ftok(FILEKEY, KEY);
	if(key == -1) {
		fprintf (stderr, "Error con la clave \n");
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}
		
	id_zone = shmget(key, sizeof(struct info), IPC_CREAT | IPC_EXCL |SHM_R | SHM_W);

	if(id_zone == -1){
		fprintf(stderr, "Error en shmget");
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}

	info = shmat(id_zone, (struct info*)0, 0);
	if(info ==  NULL){
		fprintf(stderr, "Error reservando memoria compartida\n");
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}

	pid = fork();

	if(pid < 0){
    	fprintf(stderr, "Error al crear el proceso hijo\n");
    	Borrar_Semaforo(semid);
    	shmdt((struct info*)produccion);
    	shmctl(id_zone, IPC_RMID, (struct shmid_ds*)NULL);
    	exit(EXIT_FAILURE);
 	}

	if(pid == 0){
		Down_Semaforo(semid, 0, SEM_UNDO);

		fprintf(stdout, "Letras:\n%s", produccion->letras);

		fprintf(stdout, "Numeros:\n");
		for (i = 0; i < NUMS; i++){
			fprintf(stdout, "%d\t", produccion->nums[i]);
		}

		Up_Semaforo(semid, 0, SEM_UNDO);

		exit(EXIT_SUCCESS)


	}else{
		Down_Semaforo(semid. 0, SEM_UNDO);

		for(i = 0; i < TAM_ABC; i++){
			produccion->letras[i] = 97 + i;
		}
		produccion->letras[i] = '\0';

		for(i = 0; i < NUMS; i++){
			produccion->nums[i] = i;
		}

		Up_Semaforo(semid, 0, SEM_UNDO);
	}

	waitpid(pid, status, WUNTRACED | WCONTINUED);
	Borrar_Semaforo(semid);
	shmdt((struct info*)info);
	shmctl(id_zone, IPC_RMID, (struct shmid_ds*)NULL);
	exit(EXIT_SUCCESS);
	return; 
}
