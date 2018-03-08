#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

typedef struct {
	int n;
	char cadena[100];
}Struct;


void* calcula_primos(void* estructura){
	Struct *s;
	int i,j,k;

	s = (Struct*)estructura;

	for (i = 1, j = 0; j<(int)s->n; i++){
		for(k = i-1; k>0; k--){
			if(i % k == 0) k = -1;
		}

		if(k != -1){
			j++;
		}

	}
	return NULL;
}

int main (int argc, char const *argv[]){
	Struct *estructura;
	pthread_t h[100];
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

	estructura ->n = atoi(argv[1]);

	ini = clock();

	for(i = 0; i<100; i++){
		pthread_create(&h[i], NULL , calcula_primos , (void*)estructura);
		pthread_join(h[i],NULL);
	}

	fin = clock();

	free(estructura);

	printf("Se ha tardado %lf segundos\n", (fin - ini)/CLOCKS_PER_SEC);

 	exit(EXIT_SUCCESS);
}