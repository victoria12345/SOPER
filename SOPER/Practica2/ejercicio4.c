#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

/**
* @brief main del ejercicio 4
* @param argv[0] numero de hijos que se desea crear
*
* @author Victoria Pelayo e Ignacio Rabunnal
*/
int main(int argc, char const *argv[]){
	int i;
	int pid,pid2;
	int cont = 1; 
	int total ;

	if(argc < 2){
		printf("Escribe el numero de hijos que deseas crear\n");
		return 1;
	}

	total = atoi(argv[1]);

	pid = fork();

	while(1){
		if(pid == 0){
			for (i = 0; i < 3; ++i){
				fprintf(stdout, "Soy %d y estoy trabajando\n", getpid());
				sleep(1);
			}

			kill(getppid(), SIGUSR1);

			while(1){
				fprintf(stdout, "Soy %d y estoy trabajando\n", getpid());
				sleep(1);
			}

		}else{
			void manejador_SIGUSR1();
	 		signal(SIGUSR1, manejador_SIGUSR1); /*Armar la señal */
	 		
 			pause();
 			if(cont < total){
 				pid2 = fork();

 				if(pid2 == 0){ /*Si es el hijo mata al anterior hijo*/
 					kill(pid, SIGTERM);
 					pid = pid2;
 				}else{
 					pid = pid2;
 					cont++;
 				}
 			}else{
 				/*Si ya se han creado total hijos se acaba con el ultimo y con el padre*/
 				kill(pid, SIGTERM);
 				exit(0);
 			}	
 		
		}
 	}


	return 0;
}

/**
* @brief manejador de la sennal USR1
* @param sig sennal que recibe
* @author Victoria Pelayo e Ignacio Rabunnal
*/
void manejador_SIGUSR1(int sig){

	signal(sig, SIG_IGN);

	signal(sig,manejador_SIGUSR1);

}
