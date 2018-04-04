#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/shm.h>
#include <stdlib.h>
#include "semaforos.h"

#define MAX_CAJAS 2
#define NUM 50
#define KEY 75798
#define FILEKEY "/bin/cat" 
#define CUENTA_GLOBAL "cuentaGlobal.txt"
#define FICH_HIJO "hijo.txt"

int aleatorio(int inf, int supremo){
	if(inf < 0|| supremo <0 || supremo < inf){
		return -1;
	}

	return inf + rand() % (supremo-inf+1);
}

void captura(int sennal){
	return;
}

int main(void){
	int semid1, semid2;
	FILE* pf;
	sigset_t set, aux;

	pf = fopen(CUENTA_GLOBAL, "w");
	if(!pf){
		return -1;
	}
	/*Dinero inicial en la cuenta*/
	fprintf(pf, "0" );
	fclose(pf);

	/*Creamos mascara que solo tenga desbloqueada la sennal USR1*/
	sigfillset(&set);
	sigprocmask(SIG_BLOCK, &set, &aux);
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigprocmask(SIG_UNBLOCK, &set, &aux);

	/* Creamos los dos semaforos que vamos a usar*/
	
	if(Crear_Semaforo(IPC_PRIVATE, 1, &semid1) == ERROR){
		printf("Error creando el primer semafro");
		return -1;
	}

	if(Crear_Semaforo(IPC_PRIVATE, 1, &semid2) == ERROR){
		printf("Error creando el segundo semafro");
		Borrar_Semaforo(semid1);
		return -1;
	}




	return 0;
}