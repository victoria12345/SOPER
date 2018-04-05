#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#define NUM_PROC 5

/**
* @brief capturador de una sennal
* @param sennal sennal que va a capturar
* @author Victoria Pelayo e Ignacio Rabunnal
*/
void captura(int sennal){
	printf("Soy %d y he recibido la señal SIGTERM\n", getpid());
	exit(0);
}

/**
* @brief main del ejercicio 6b
* 
* @author Victoria Pelayo e Ignacio Rabunnal
*/
int main (void)
{

	int pid, counter;
	pid = fork();

	if (pid == 0){
		while(1){

			if(signal(SIGTERM,captura) == SIG_ERR){
				printf("Error capturando el\n");
				exit(EXIT_FAILURE);
			}

			for (counter = 0; counter < NUM_PROC; counter++){
				printf("%d\n", counter);
				sleep(1);
			}
			sleep(3);
		}
	}else{

		sleep(40);
		kill(pid,SIGTERM);
		wait(NULL);
		exit(0);
	}
	

} 

