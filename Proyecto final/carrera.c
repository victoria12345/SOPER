/************************************************
* @file carrera.c fichero de la carrera de caballos
*
* @author Victoria Pelayo e Ignacio 
* @date 27-04-2018
**/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <time.h>
#include "semaforos.h"

#define FILEKEY "/bin/cat"
#define KEY 2345

#define SEMKEY 2759

#define MAX_WORD 200

#define DADO_NORMAL 1
#define SIETE_CARAS 2
#define DOBLE_DADO 3

#define ACABADA 4
#define EN_PROCESO 5


typedef struct _Mensaje{
	long id; /*!< Tipo del mensaje*/
	int tirada; /*!< Resultado de tirar el dado*/
}Mensaje;

/**
* Se activa cuando el caballo tiene que seguir corriendo
* Simplemente el caballo sigue con la carrera
* @param senna sennal que recibe
*/
void manejador_carrera(int sennal){
}

/**
* Se activa cuando el caballo tiene que terminal la carrera
* @param sennal sennal que recibe
*/
void manejador_final(int sennal){
	exit(EXIT_SUCCESS);
}

/**
* Funcion que simula el comportamiento del caballo
*/
int caballo(int tipo_dado, int id_mensajes, int pos){
	Mensaje msj;

	if(id_mensajes < 0 || pos < 0){
		return -1;
	}

	if(tipo_dado == DADO_NORMAL){
		msj.tirada = rand() %6 +1;
	}else if(tipo_dado == DOBLE_DADO){
		msj.tirada = (rand() %6 +1) + (rand() % 6 +1);
	}else{
		msj.tirada = rand() %7 +1;
	}

	msj.id = pos + 2;
	msgsnd(id_mensajes, (struct msgbuf*)&msj, sizeof(Mensaje)-sizeof(long), IPC_NOWAIT);

	return 0;
}

/**
* Calcula la posicion del caballo para saber que tirada le toca
* @param array array de las posiciiones de los caballos
* @param pos posicion del caballo que interesa
* @param longitud longitud del array
*
* @return 1 tirada normal, 0 si es dado del 1 al 7, 2 si es dado remontador
*/
int calcular_tirada(int* array, int pos, int longitud){
	int i;
	int primero = array[0];
	int ultimo = array[0];

	if(array == NULL || pos < 0 || pos > longitud || longitud < 0){
		return -1;
	}

	for(i = 0; i < longitud; i++){
		if(array[i] > primero){
			primero = array[i];
		}
		if(array[i] < ultimo){
			ultimo = array[i];
		}
	}
	/*Dado especial de 7*/
	if(primero == array[pos]){
		return SIETE_CARAS;
	}
	/*dos dados normales*/
	if(ultimo == array[pos]){
		return DOBLE_DADO;
	}
	/*Dado normal*/
	return DADO_NORMAL;
}

int main(int argc, char const *argv[]){

	int estado = 0;

	int n_caballos, longitud, n_apostadores, n_ventanillas, cantidad;
	int i, j, n_semaforos = 3;

	key_t clave;
	int id_mensajes, semid;

	int pid_apostador, pid_gestor;

	char *mensaje;

	char* apostadores;

	int *pids, *posicion;

	unsigned short *array;

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

	if((apostadores = (char*)malloc(sizeof(char) * n_apostadores)) == NULL){
		printf("Error al reservar memoria para el array de apostadores\n");
		exit(EXIT_FAILURE);
	}

	for(i = 0; i < n_apostadores ; i++){
		sprintf(apostadores[i], "Apostador-%d", i+1);
	}

		/*Reservamos memoria para el mensaje*/
	if((mensaje = (char*)malloc(sizeof(char) * MAX_WORD)) == NULL){
		printf("Error reservando memoria para el mensaje");
		free(apostadores);
		exit(EXIT_FAILURE);
	}

		/*Creamos la zona de memoria compartida*/
	clave = ftok(FILEKEY, KEY);
	if(clave == (key_t)-1){
		printf("Error al obtener la clave de la cola de mensajes");
		free(mensaje);
		free(apostadores);
		exit(EXIT_FAILURE);
	}

	id_mensajes = msgget(clave, 0666 | IPC_CREAT);

	if(id_mensajes == -1){
		printf("Error con el id de la cola\n");
		free(mensaje);
		free(apostadores);
		exit(EXIT_FAILURE);
	}

		/*Creamos tantas tuberias como caballos*/
	for(i = 0; i < n_caballos; i++){
		if(pipe(pipes[i]) == -1){
			printf("Error creando las tuberias \n");
			free(mensaje);
			free(apostadores);
			exit(EXIT_FAILURE);
		}
	}

		/*Creamos los semaforos*/
	if(Crear_Semaforo(SEMKEY, n_semaforos, &semid) == ERROR){
		printf("Error creando los semaforos\n");
		free(mensaje);
		free(apostadores);
		exit(EXIT_FAILURE);
	}

	array = (unsigned short*)malloc(sizeof(unsigned short) * n_semaforos);
	if(array == NULL){
		printf("Error reservando memoria para el array");
		free(mensaje);
		free(apostadores);
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}

	array[0] = 1;
	array[1] = 1;
	array[2] = 0;

	if(Inicializar_Semaforo(semid, array) == ERROR){
		printf("Error al incializar el semaforo\n");
		free(mensaje);
		free(apostadores);
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}

	pids = (int*)malloc(sizeof(int) * n_caballos);

	if(pids == NULL){
		printf("Error al incializar el semaforo\n");
		free(mensaje);
		free(apostadores);
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}

	posicion = (int*)malloc(sizeof(int) * n_caballos);

	if(posicion == NULL){
		printf("Error al incializar el semaforo\n");
		free(mensaje);
		free(pids);
		free(apostadores);
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}

	/*CREAMOS EL PROCESO MONITOR*/


	/*CREAMOS EL PROCESO GESTOR DE APUESTAS*/
	pid_gestor = fork();
	if(pid_gestor == 0){

	}

	/*CREAMOS EL PROCESO APOSTADOR*/
	pid_apostador = fork();
	if(pid_apostador == 0){
		
	}



	/*CREAMOS LOS CABALLOS*/

	for(i = 0 ; i < n_caballos; i++){
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

			sleep(2);
			/*envia informacion sobre posicion*/
			sprintf(mensaje, "%d", DADO_NORMAL);
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
					msgrcv(id_mensajes, (struct msgbuf*) &msj, sizeof(Mensaje) - sizeof(long), j+2, 0);
					posicion[j] += msj.tirada;
					if(posicion[j] >= longitud){
						estado  = ACABADA;
					}
				}
				/*Indica el tipo de dado que utilizaran*/
				for(j = 0; j < n_caballos; j++){
					sprintf(mensaje, "%d", calcular_tirada(posicion, j, n_caballos));
					close(pipes[j][0]);
					write(pipes[j][1], mensaje, sizeof(mensaje));

					kill(pids[j], SIGUSR1);
					printf("Caballo %d: %d\n", j+1, posicion[j]);
				}
				printf( "---------------------------\n");
			}

			/*Cuando un caballo acaba se envia sennal para que todos acaben la carrera*/
			for(j = 0; j < n_caballos; j++){
				kill(pids[j], SIGUSR2);
			}
		}
		/*Codigo de los caballos*/
		else if(pids[i] == 0){

			printf("HIJO CREADO\n");

			/*Para que al lanzar los dados no todos tengan lo mismo*/
			srand(getpid());

			if(signal(SIGUSR1, manejador_carrera) == SIG_ERR){
				printf("Error con el manejador de la carrera\n");
				exit(EXIT_FAILURE);
			}

			if(signal(SIGUSR2, manejador_final) == SIG_ERR){
				printf("Error con el manejador de fin de carrera");
				exit(EXIT_FAILURE);
			}

			while(1){
				int tipo_dado;
				pause();
				/*Comunicacion con gestor de apuestas y monitor
				memset(mensaje, 0, MAX_WORD);*/

				/*Leemos mensaje de la tuberia y llamamos a la funcion de los caballos*/
				close(pipes[i][1]);
				if(read(pipes[i][0], mensaje, sizeof(mensaje)) == -1){
					printf("Error leyendo el tipo de tirada");
					exit(EXIT_FAILURE);
				}

				tipo_dado = atoi(mensaje);
				if(tipo_dado == -1){
					printf("Error con el tipo de dado\n");
					exit(EXIT_FAILURE);
				}

				if(caballo(tipo_dado, id_mensajes,i) == -1){
					printf("Error con el caballo %d durante la carrera", i+1);
					exit(EXIT_FAILURE);
				}
			}
		}
	}

	for(i = 0; i < n_caballos; i++){
		waitpid(pids[i], NULL, WUNTRACED | WCONTINUED);
	}

	exit(EXIT_SUCCESS);
}