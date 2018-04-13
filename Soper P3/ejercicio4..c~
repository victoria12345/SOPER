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

#define SEMKEY 12345

int main(int argc, char const *argv[]){
	int semid = 0;

	if(Crear_Semaforo(SEMKEY,1, &semid) == ERROR){
    	printf("Error creando el primer semaforo\n");
    	exit(ERROR);
  	} 
	return; 
}