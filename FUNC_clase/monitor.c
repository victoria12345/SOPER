#include "monitor.h"

void Iniciar_monitor(int semid, int n_caballos, Compartida *compartida){
	int i;

	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

	for(i = 0; i < TIEMPO; i++){
		int j;

		sleep(1);
		printf("Faltan %d segundos para que comience la carrera\n", TIEMPO -i);
		fflush(stdout);

		Down_Semaforo(semid, 0, SEM_UNDO);
		for(j = 0; j < n_caballos; j++){
			printf("Cotizacion caballo %d : %f\n", j+1, compartida->cotizacion[j]);
			fflush(stdout);

		}
		Up_Semaforo(semid, 0, SEM_UNDO);

		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		fflush(stdout);
	}

}

void monitor_enproceso(int semid, int id_zone2, int *estado, int longitud, int n_caballos,Compartida_monitor* compartida, int n_apostadores){
	int i;

	printf("\n\n");
	printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

	while(*estado == EN_PROCESO){

		Down_Semaforo(semid,3,SEM_UNDO);

		for(i = 0; i < n_caballos; i++){
			fprintf(stdout,"Ultima tirada caballo %d: %d. Posicion: %d\n", i+1, compartida->posicion[i+MAX_CABALLOS], compartida->posicion[i]);
			fflush(stdout);
			if(compartida->posicion[i] >= longitud){
				*estado = ACABADA;
			}
		}
		kill(getppid(), SIGUSR1);
		printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	}
	printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}
