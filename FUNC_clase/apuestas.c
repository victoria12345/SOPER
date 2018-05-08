#include "apuestas.h"

int Inicializar_apuestas(int n_caballos, Compartida* compartida){
	int i;

	
	compartida->total_apuestas = n_caballos;

	if(compartida->apuestas == NULL || compartida->cotizacion == NULL){
		return -1;
	}

	for(i = 0; i < n_caballos;i++){
		compartida->apuestas[i] = 1.0;
		compartida->cotizacion[i] = 1.0 / compartida->total_apuestas;
	}

	return 0;
}

void apostador(int id, int n_caballos, int cantidad, int id_mensajes){
	Apuesta apuesta;

	apuesta.id = 1;
	apuesta.caballo = rand() % n_caballos;
	apuesta.apuesta = rand()%cantidad + 1;
	sprintf(apuesta.apostador, "Apostador-%d", id);

	msgsnd(id_mensajes, (struct msgbuf*)&apuesta, sizeof(apuesta)-sizeof(long), IPC_NOWAIT);
}

int Inicializar_Gestor(Estructura_hilo* estructura_hilo,int semid,int n_sem,int n_caballos,int id_zone,int id_mensajes,Compartida* compartida){
	int i;

	if(estructura_hilo == NULL || compartida == NULL){
		return -1;
	}

 estructura_hilo->n_caballos = n_caballos;
 estructura_hilo->id_zone = id_zone;
 estructura_hilo->id_mensajes= id_mensajes;

 Down_Semaforo(semid,n_sem,SEM_UNDO);

 for(i = 0; i < n_caballos; i++){
	 compartida->apuestas[i] = 1.0;
 }

 compartida->total_apuestas = n_caballos;

 for(i = 0; i < n_caballos; i++){
	 compartida->cotizacion[i] = compartida->total_apuestas/compartida->apuestas[i];
 }

 Up_Semaforo(semid, n_sem, SEM_UNDO);

 return 0;
}
