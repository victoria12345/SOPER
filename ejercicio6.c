#include <stdio.h>
#include <stdlib.h>

typedef struct{
	int i;
	char cadena[80];
}estructura;

int main (void){
	int pid;
	int i;
	estructura *est;

 	if ((pid=fork()) < 0 ){
 		printf("Error haciendo fork\n");
 		exit(EXIT_FAILURE);

 	}else if (pid ==0){
 		printf("Soy el hijo %d y mi padre es %d\n", getpid(), getppid());
 	}else{
 		printf ("Soy el padre %d\n", getpid());
 		est = (estructura*)malloc(sizeof(estructura));
 		free(est);
 	}

}