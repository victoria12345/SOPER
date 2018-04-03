#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#define NUM_PROC 5

int main (void){
int pid, counter;
sigset_t set;

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

			for (counter = 0; counter < NUM_PROC; counter++){
				printf("%d\n", counter);
				sleep(1);
			}

			if(sigismember(&set,SIGUSR1) == 1){
				sigdelset(&set,SIGUSR1);
			}
			if(sigismember(&set,SIGALRM) == 1){
				sigdelset(&set,SIGALRM);
			}

		sleep(3);
		}
	}
	while(wait(NULL)>0);

	return 0;
}