#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#define NUM_PROC 5

void captura(int sennal){
	printf("Soy %d y he recibido la señal SIGTERM\n", getpid());
	exit(0);
}


int main (void)
{

	int pid, counter;
	pid = fork();

	if (pid == 0){
		while(1){

			signal(SIGTERM,captura);

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

