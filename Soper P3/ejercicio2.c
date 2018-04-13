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
#define FILEKEY "/bin/cat"
#define KEY 2345

struct info{
		char nombre[80];
		int id;
};

void manejador_SIGUSR1(int sig);

int main(int argc, char const *argv[]){
	int i;
	int total;
	struct info *info;
	int key, id_zone;
	int *status = NULL;

	if(argc < 2){
		printf("Escribe el numero de hijos que deseas crear\n");
		return 1;
	}

	key = ftok(FILEKEY, KEY);
	if(key == -1) {
		fprintf (stderr, "Error con la clave \n");
		return -1; 
	}
		
	id_zone = shmget(key, sizeof(struct info), IPC_CREAT | IPC_EXCL |SHM_R | SHM_W);

	if(id_zone == -1){
		fprintf(stderr, "Error en shmget");
	}

	info = shmat(id_zone, (struct info*)0, 0);
	if(info ==  NULL){
		fprintf(stderr, "Error reservando memoria compartida\n");
		return -1;
	}

	info->id = 0;	

	total = atoi(argv[1]);


	int childpid[total]; 
	int contadorHijos = total;

	for(i = 0; i < total; i++){

		childpid[i] = fork();

		if(childpid[i] == 0){
			sleep(rand()%5);

			fprintf(stdout, "Introduzca el nombre del nuevo cliente:\n");
			fscanf(stdin,"%s",info->nombre);

			fprintf(stdout, "El último id ha sido: %d\n", info->id);
			info->id++;

			kill(getppid(), SIGUSR1);
			exit(EXIT_SUCCESS);
		}else{

	 		if(signal(SIGUSR1, manejador_SIGUSR1) == SIG_ERR){
	 			fprintf(stderr, "Error en la captura");
	 			exit(EXIT_FAILURE);
	 		};

			if(i < total - 1){
				continue;
			}
		

	 		while(contadorHijos != 0){

		 		pause();
		 		contadorHijos--;

				fprintf(stdout, "Nombre: %s\nID: %d\n", info->nombre, info->id);	
			}
		}
	}

	for(i = 0; i < total; i++){
		waitpid(childpid[i], status, WUNTRACED | WCONTINUED);
	}

	shmdt((struct info*)info);
	shmctl(id_zone, IPC_RMID, (struct shmid_ds*)NULL);
	exit(EXIT_SUCCESS);

}


/**
* @brief manejador de la sennal USR1
* @param sig sennal que recibe
* @author Victoria Pelayo e Ignacio Rabunnal
*/
void manejador_SIGUSR1(int sig){
	return;
}