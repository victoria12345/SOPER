/****************************************
*
* @file ejercicio13.c
*
* @author Ignacio Rabuñal García
* @author Victoria Pelayo Alvaredo
*
* @date 08/03/2018
*
* @brief Se piden por linea de comandos dos matrices y
*dos escalares y, mediante dos hilos de ejecución, se
*multiplia cada matriz por su respectivo escalar.
*
*****************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>


#define MAX_CHAR 256

//Definimos dos variables globales para que sea posible la comunicación entre los hilos
int fila_h1 = 0;
int fila_h2 = 0;

//Estructura que contiene toda la información necesaria para la ejecución del hilo
typedef struct{
	int *matriz;
	int dim;
	int escalar;
	char nombre[MAX_CHAR];
	pthread_t hilo;
}Param;


void *multiplicacion_escalar(void *param);

int main(int argc, char const *argv[]){
	int dim, i;
	pthread_t hilo1, hilo2;

	Param *param1 =(Param*)malloc(sizeof(Param));
	Param *param2 =(Param*)malloc(sizeof(Param));

	printf("Introduzca dimension de la matriz cuadrada:\n");
	scanf("%d", &dim);


	if(dim > 5){
		printf("La dimension de la matriz no puede ser mayor que 5");
		exit(EXIT_FAILURE);
	}

	strcpy(param1->nombre, "Hilo1");
	strcpy(param2->nombre, "Hilo2");

	param1->matriz = (int*)malloc(dim*dim*sizeof(int));
	param2->matriz = (int*)malloc(dim*dim*sizeof(int));

	param1->dim = dim;
	param2->dim = dim;

	printf("Introduzca multiplicador 1:\n");
	scanf("%d", &(param1->escalar));
	printf("Introduzca multiplicador 2:\n");
	scanf("%d", &(param2->escalar));

	printf("Introduzca matriz 1:\n");
	for(i = 0; i < dim*dim; i++){
		scanf("%d", &(param1->matriz[i]));
	}

	printf("Introduzca matriz 2:\n");

	for(i = 0; i < dim*dim; i++){
		scanf("%d", &(param2->matriz[i]));
	}

	//Creamos los hilos
	pthread_create (&(param1->hilo), NULL, (void*)multiplicacion_escalar, (void*)&param1);
 	pthread_create (&(param2->hilo), NULL, (void*)multiplicacion_escalar, (void*)&param2);

	//Terminamos su ejecución
 	pthread_join(param1->hilo,NULL);
 	pthread_join(param2->hilo,NULL);

	//Liberamos recursos
	free(param1->matriz);
	free(param2->matriz);
	free(param1);
	free(param2);

	return 0;
}


void *multiplicacion_escalar(void *param){
	int i, j, dim;
	char frase[256] = "\0";
	char frase_aux[256] = "\0";

	dim = ((Param*)param)->dim;

	if(strcmp("Hilo2",((Param*)param)->nombre) == 0){
		sleep(5);
	}

	for (i = 0; i < dim; i++){

		sprintf(frase, "%s multiplicando fila %d resultado", ((Param*)param)->nombre, i);

		//Realizamos la multiplicación de la matriz por el escalar
		for(j = i*dim; j < i*dim + dim; j++){
			sprintf(frase_aux, "%d", ((Param*)param)->matriz[j]*((Param*)param)->escalar);
			strcat(frase, frase_aux);
		}

		//Comprueba si estamos en el primer hilo y si es así, imprimimos el estado del segundo hilo
		if(strcmp("Hilo1", ((Param*)param)->nombre) == 0){
			fila_h1++;
			if(fila_h2 == dim){
				sprintf(frase_aux, "- el Hilo2 ha terminado");
			}else{
				sprintf(frase_aux, "- el Hilo2 va por la fila %d", fila_h2);
			}

			strcat(frase, frase_aux);
			fprintf(stdout, "%s\n", frase);
			sleep(5);

		//Comprueba si estamos en el segundo hilo y si es así, imprmimos el estado del primer hilo
		}else if(strcmp("Hilo2", ((Param*)param)->nombre) == 0){
			fila_h2++;
			if(fila_h1 == dim){
				sprintf(frase_aux, "- el Hilo1 ha terminado");
			}else{
				sprintf(frase_aux, "- el Hilo1 va por la fila %d", fila_h1);
			}

			strcat(frase, frase_aux);
			fprintf(stdout, "%s\n", frase);
			sleep(3);

		}
	}

	pthread_exit(NULL);

}
