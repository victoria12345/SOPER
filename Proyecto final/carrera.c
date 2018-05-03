/************************************************
* @file carrera.c fichero de la carrera de caballos
*
* @author Victoria Pelayo e Ignacio 
* @date 27-04-2018
**/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#define FILEKEY "/bin/cat"
#define KEY 2345

#define SEM_KEY 2759

#define ACABADA 1
#define EN_PROCESO 2

typedef struct _Mensaje{
	long id; /*!< Tipo del mensaje*/
	int tirada; /*!< Resultado de tirar el dado*/
}Mensaje;

/**
* Calcula la posicion del caballo para saber que tirada le toca
* @param array array de las posiciiones de los caballos
* @param pos posicion del caballo que interesa
* @param max numero de caballos
*
* @return 1 tirada normal, 0 si es dado del 1 al 7, 2 si es dado remontador
*/
int calcular_tirada(int* array, int pos, int max){

}

int main(int argc, char const *argv[]){

	int estado = 0;

	int n_caballos, longitud, n_apostadores, n_ventanillas, cantidad;
	int i, j,n_semaforos = 3;

	key_t clave;
	int id_mensajes, semid;

	char *mensaje;

	int *pids, *posicion;

	int *array;

	Mensaje msj;

	if(argc < 6){

		printf("Faltan por introducir datos para la carrera: \n"
			"-(int)Numero de participantes(maximo 10)\n"
			"-(int) longitud de la carrera\n"
			"-(int) Numero apostadores (maximo 100)\n"
			"-(int) Numero ventanillas para gestionar las apuestas\n"
			"-(int) Cantidad disponible de dinero de cada apostante\n\n");
		exit(EXIT_FAILURE);
	}

	if(atoi(argv[1]) > 10|| atoi(argv[1]) <= 0|| atoi(argv[3]) > 100|| atoi(argv[3]) <= 0|| atoi(argv[5]) < 0){

		printf("Error con los parametros introducidos\n");

		printf(
			"-(int)Numero de participantes(maximo 10)\n"
			"-(int) longitud de la carrera\n"
			"-(int) Numero apostadores (maximo 100)\n"
			"-(int) Numero ventanillas para gestionar las apuestas\n"
			"-(int) Cantidad disponible de dinero de cada apostante\n\n");
		exit(EXIT_FAILURE);
	}

	n_caballos = atoi(argv[1]);
	longitud = atoi(argv[2]);
	n_apostadores = atoi(argv[3]);
	n_ventanillas = atoi(argv[4]);
	cantidad = atoi(argv[5]);

	int pipes[n_caballos][2];

		/*Reservamos memoria para el mensaje*/
	if((mensaje = (char*)malloc(sizeof(char) * 200)) == NULL){
		printf("Error reservando memoria para el mensaje");
		exit(EXIT_FAILURE);
	}

		/*Creamos la zona de memoria compartida*/
	clave = ftok(FILEKEY, KEY);
	if(clave == (key_t)-1){
		printf("Error al obtener la clave de la cola de mensajes");
		free(mensaje);
		exit(EXIT_FAILURE);
	}

	id_mensajes = msgget(clave, 0666 | )

		/*Creamos tantas tuberias como caballos*/
	for(i = 0; i < n_caballos; i++){
		if(pipe(pipes[i]) == -1){
			printf("Error creando las tuberias \n");
			free(mensaje);
			exit(EXIT_FAILURE);
		}
	}

		/*Creamos los semaforos*/
	if(Crear_Semaforo(SEMKEY, 3, &semid) == ERROR){
		printf("Error creando los semaforos\n");
		free(mensaje);
		exit(EXIT_FAILURE);
	}

	array = (unsigned short*)malloc(sizeof(unsigned short) * 3);
	if(array == NULL){
		printf("Error reservando memoria para el array");
		free(mensaje);
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}

	array[0] = 1;
	array[1] = 1;
	array[2] = 0;

	if(Inicializar_Semaforo(semid, array) == ERROR){
		printf("Error al incializar el semaforo\n");
		free(mensaje);
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}

	pids = (int*)malloc(sizeof(int) * n_caballos);

	if(pids == NULL){
		printf("Error al incializar el semaforo\n");
		free(mensaje);
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}

	posicion = (int*)malloc(sizeof(int) * n_caballos);

	if(posicion == NULL){
		printf("Error al incializar el semaforo\n");
		free(mensaje);
		free(pids);
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}

		/*CREAMOS EL PROCESO MONITOR*/

		/*CREAMOS EL PROCESO GESTOR DE APUESTAS*/

		/*CREAMOS EL PROCESO APOSTADOR*/

		/*CREAMOS LOS CABALLOS*/

	for(i= 0 ; i < n_caballos; i++){
			/*Creamos tantos hijos como caballos*/
		pids[i] = fork();

		if(pids[i] < 0){
				/*FALTAN CONTROLES DE ERORRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR*/
			printf("Error creando un hijo");
			Borrar_Semaforo(semid);
			free(mensaje);
			exit(EXIT_FAILURE);
		}
		else if(pids[i] > 0){
				/*Envia posicion a los hijos*/
			sprintf(mensaje, "%d", 1);
			close(pipes[i][0]);
			write(pipes[i][1], mensaje, strlen(mensaje));

			kill(pids[i], SIGUSR1);

				/*Cuando no se han creado todos los caballos*/
			if(i < n_caballos -1){
				continue;
			}

			while(estado != ACABADA){
				/*Actualizamos las posiciones de los caballos*/
				for(j = 0; j < n_caballos; j++){
					/*El id 1 esta ocupado con el gestor de apuestas*/
					msgrcv(mensaje, (struct msgbuf*) &msj, sizeof(Mensaje) - sizeof(long), j+2, 0);
					posicion[j] += msj.tirada;
					if(posicion[j] >= longitud){
						estado  = ACABADA;
					}
				}

				for(j = 0; j < n_caballos; j++){
					sprintf(mensaje, "%d", calcular_tirada(posicion, j n_caballos));
					close(pipes[j][0]);
					write(pipes[j][1], mensaje, strlen(mensaje));

					
				}
			}
		}
	}



	exit(EXIT_SUCCESS);
}