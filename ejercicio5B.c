#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define NUM_PROC 6
int main (void){
	int pid;
	int i;
	int *status = NULL;
 	for (i=0; i <= NUM_PROC; i++){
 		if (i % 2 != 0) {
 			if ((pid=fork()) <0 ){
 				printf("Error haciendo fork\n");
 				exit(EXIT_FAILURE);
 			}else if (pid ==0){
 				printf("Soy el hijo %d y mi padre es %d\n", getpid(), getppid());
 				exit(EXIT_SUCCESS);
 			}else{
 				printf ("PADRE %d con hijo %d\n", getpid(), pid);
 			}
 		}
 	}
 	wait(status);
 	exit(EXIT_SUCCESS);
}