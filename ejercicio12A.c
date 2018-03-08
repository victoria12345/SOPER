#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

typedef struct {
	int n;
	char cadena[100];
}Struct;


void calcula_primos(int N){
	int i,j,k;

	for (i = 1, j = 0; j< N; i++){
		for(k = i-1; k>1; k--){
			if(i % k == 0) k = -1;
		}

		if(k != -1){
			j++;
		}
	}

}

int main (int argc, char const *argv[]){
	Struct *estructura;
	pid_t child_pid;
	int status;
	double ini, fin;
	int i;

	if(argc < 2){
		printf("Introduce el numero de primos a calcular\n");
		return -1;
	}

	estructura = (Struct*)malloc(sizeof(Struct));

	if(estructura == NULL){
		printf("Error reservando memoria");
		return -1;
	}

	ini = clock();

	for(i = 0; i<100; i++){
		child_pid = fork();
		if(child_pid != 0){
			wait(&status);
		}else{
			calcula_primos(atoi(argv[1]));
			exit(EXIT_SUCCESS);
		}
	}

	fin = clock();

	free(estructura);

	printf("Se ha tardado %lf segundos\n", (fin - ini)/(double)CLOCKS_PER_SEC);

 	exit(EXIT_SUCCESS);
}
