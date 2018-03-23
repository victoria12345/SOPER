#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>



int main(int argc, char const *argv[]){
	int i;
	int pid;
	int ppid;

	pid = fork();
	if(pid == 0){
		for (i = 0; i < 10; ++i){
			fprintf(stdout, "Soy %d y estoy trabajando\n", getpid());
			sleep(1);
		}

		kill(getppid(), SIGUSR1);

		while(1){
			fprintf(stdout, "Soy %d y estoy trabajando\n", getpid());
			sleep(1);
		}
      

	}else{
		ppid = getpid();
		void manejador_SIGUSR1();
 		signal(SIGUSR1, manejador_SIGUSR1); /*Armar la señal */
 		while(1){
 			if(pause() == -1){
 				if(ppid == getppid()){
 					break;
 				}
 			}
 		}

	}
	return 0;
}


void manejador_SIGUSR1(int sig){

	signal(sig, SIG_IGN);

	fork();

	signal(sig, ,manejador_SIGUSR1);

}
