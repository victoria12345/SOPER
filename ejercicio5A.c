#include <stdio.h>
#include <stdlib.h>
#define NUM_PROC 6
int main (void){
	int pid;
	int i;
 	for (i=0; i <= NUM_PROC; i++){
 		if (i % 2 == 0) {
 			if ((pid=fork()) <0 ){
 				printf("Error haciendo fork\n");
 				exit(EXIT_FAILURE);
 			}else if (pid ==0){
 				printf("Soy el hijo %d y mi padre es %d\n", getpid(), getppid());
 			}else{
 				printf ("PADRE %d\n", i);
 			}
 		}else if (i % 2 != 0) {
 			if ((pid=fork()) <0 ){
 				printf("Error haciendo fork\n");
 				exit(EXIT_FAILURE);
 			}else if (pid ==0){
 				printf("Soy el hijo %d y mi padre es %d\n", getpid(), getppid());
 			}else{
 				printf ("PADRE %d\n", i);
 				wait();
 			}
 	}
 	wait();
 	exit(EXIT_SUCCESS);
}