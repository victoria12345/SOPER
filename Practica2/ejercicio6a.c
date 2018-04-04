#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#define NUM_PROC 5

/**
* @brief main del ejercicio 6a
* Comprobamos que el hijo recibe la sennal de alarma justo antes de dormir 3 segundos
* cuando la desbloquea
*
* @author Victoria Pelayo e Ignacio Rabunnal
*/
int main (void){
int pid, counter;
sigset_t set, aux;

sigemptyset(&set);

pid = fork();

	if (pid == 0){

		alarm(40);

		while(1){
			
			/**las annado al set si no estan*/
			if(sigismember(&set,SIGUSR1) == 0){
				sigaddset(&set,SIGUSR1);
			}
			if(sigismember(&set,SIGUSR2) == 0){
				sigaddset(&set,SIGUSR2);
			}
			if(sigismember(&set,SIGALRM) == 0){
				sigaddset(&set,SIGALRM);
			}

			if(sigprocmask(SIG_BLOCK, &set, &aux)){
				printf("Error al bloquear las sennales");
			}

			for (counter = 0; counter < NUM_PROC; counter++){
				printf("%d\n", counter);
				sleep(1);
			}

			if(sigismember(&set,SIGUSR2) == 1){
				sigdelset(&set,SIGUSR2);
			}

			if(sigprocmask(SIG_UNBLOCK, &set, &aux)){
				printf("Error al desbloquear las sennales\n");
			}

			sleep(3);
		}
	}
	while(wait(NULL)>0);

	return 0;
}