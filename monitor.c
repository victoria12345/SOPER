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