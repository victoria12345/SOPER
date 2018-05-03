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
#include <pthread.h>
#include <math.h>
#include <fcntl.h>
#include "semaforos.h"
#include "caballos.h"
#include "apuestas.h"

#define FILEKEY "/bin/cat"
#define KEY 8567

#define SEMKEY 7605

#define MAX_WORD 200

#define ACABADA 4
#define EN_PROCESO 5
#define ANTES 6

int estado = ANTES;
unsigned short *array;
int n_semaforos = 3;
int semid;

void* ventanilla(void* estructura_hilo){
	int id;
	char *tmp;

	Apuesta apuesta;
	Compartida* compartida;

	FILE *f;

	if(estructura_hilo == NULL){
		pthread_exit(NULL);
	}

	id = ((Estructura_hilo*)estructura_hilo)->id;

	Up_Semaforo(semid,2,SEM_UNDO);

}


int main(int argc, char const *argv[]){


	int n_caballos, longitud, n_apostadores, n_ventanillas, cantidad;
	int i, j;

	key_t clave;
	int id_mensajes, semid;

	int pid_apostador, pid_gestor;

	char *mensaje;

	char** apostadores;

	int *pids, *posicion;

	pthread_t* hilo;
	Estructura_hilo* estructura_hilo;

	/*Para memoria compartida*/
	Compartida* compartida;
	key_t clave_mem_comp;
	int id_zone;


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

	if((apostadores = (char**)malloc(sizeof(char*) *( n_apostadores + 1))) == NULL){
		printf("Error al reservar memoria para el array de apostadores\n");
		exit(EXIT_FAILURE);
	}

	for(i = 0; i < n_apostadores ; i++){
		apostadores[i] = (char*)malloc(sizeof(char) * MAX_WORD);
		sprintf(apostadores[i], "Apostador-%d", i+1);
	}

		/*Reservamos memoria para el mensaje*/
	if((mensaje = (char*)malloc(sizeof(char) * MAX_WORD)) == NULL){
		printf("Error reservando memoria para el mensaje");
		for(i = 0; i < n_apostadores; i++){
			free(apostadores[i]);
		}
		free(apostadores);
		exit(EXIT_FAILURE);
	}

		/*Creamos la zona de memoria compartida*/
	clave = ftok(FILEKEY, KEY);
	if(clave == (key_t)-1){
		printf("Error al obtener la clave de la cola de mensajes");
		free(mensaje);
		for(i = 0; i < n_apostadores; i++){
			free(apostadores[i]);
		}
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
			for(int i = 0; i < n_apostadores; i++){
				free(apostadores[i]);
			}
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
		for(int i = 0; i < n_apostadores; i++){
			free(apostadores[i]);
		}
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
		for(int i = 0; i < n_apostadores; i++){
			free(apostadores[i]);
		}
		free(apostadores);
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}

	pids = (int*)malloc(sizeof(int) * n_caballos);

	if(pids == NULL){
		printf("Error al incializar el semaforo\n");
		free(mensaje);
		for(int i = 0; i < n_apostadores; i++){
			free(apostadores[i]);
		}
		free(apostadores);
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}

	posicion = (int*)malloc(sizeof(int) * n_caballos);

	if(posicion == NULL){
		printf("Error al incializar el semaforo\n");
		free(mensaje);
		free(pids);
		for(int i = 0; i < n_apostadores; i++){
			free(apostadores[i]);
		}
		free(apostadores);
		free(posicion);
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}

	/*Creamos la zona de memoria compartida*/

	clave_mem_comp = ftok(FILEKEY,KEY);
	if(clave_mem_comp == -1){
		printf("Error con el ftok de la memorica compartida\n");
		free(mensaje);
		free(pids);
		for(int i = 0; i < n_apostadores; i++){
			free(apostadores[i]);
		}
		free(apostadores);
		free(posicion);
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}

	id_zone = shmget(clave_mem_comp,sizeof(struct _Compartida),IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
	if(id_zone == -1){
		printf("Error con el id de la zona de memoria compartida\n");
		shmctl(id_zone,IPC_RMID,(struct shmid_ds*)NULL);
		free(mensaje);
		free(pids);
		for(int i = 0; i < n_apostadores; i++){
			free(apostadores[i]);
		}
		free(apostadores);
		free(posicion);
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}

	printf("Id_zone: %d\n", id_zone);

	compartida = shmat(id_zone, (struct Compartida*)0,0);
	if(compartida == NULL){
		printf("Error reservando memorica para compartida\n");
		shmctl(id_zone,IPC_RMID,(struct shmid_ds*)NULL);
		free(mensaje);
		free(pids);
		for(int i = 0; i < n_apostadores; i++){
			free(apostadores[i]);
		}
		free(apostadores);
		free(posicion);
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}

	hilo = (pthread_t*)malloc(sizeof(pthread_t)*n_ventanillas);
	if(hilo == NULL){
		printf("Error al reservar memoria para el hilo\n");
		shmdt((struct _Compartida*)compartida);
		shmctl(id_zone,IPC_RMID,(struct shmid_ds*)NULL);
		free(mensaje);
		free(pids);
		for(int i = 0; i < n_apostadores; i++){
			free(apostadores[i]);
		}
		free(apostadores);
		free(posicion);
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}

	/*Los caballos no han comenzado*/
	for(i = 0; i < n_caballos; i++){
		posicion[i] = 0;
	}

	/*CREAMOS EL PROCESO MONITOR*/


	/*CREAMOS EL PROCESO GESTOR DE APUESTAS*/

	estructura_hilo = (Estructura_hilo*)malloc(sizeof(Estructura_hilo*));

	if(estructura_hilo == NULL){
		printf("Error reservando memoria para la estructura_hilo\n");
		shmdt((struct _Compartida*)compartida);
		shmctl(id_zone,IPC_RMID,(struct shmid_ds*)NULL);
		free(mensaje);
		free(pids);
		for(int i = 0; i < n_apostadores; i++){
			free(apostadores[i]);
		}
		free(apostadores);
		Borrar_Semaforo(semid);
		exit(EXIT_FAILURE);
	}

	pid_gestor = fork();
	if(pid_gestor == 0){
		if(signal(SIGUSR1, manejador_carrera) == SIG_ERR){
			printf("Error con el manejador_carrera en gestor \n");
			exit(EXIT_FAILURE);
		}

		Down_Semaforo(semid, 0, SEM_UNDO);
		if(Inicializar_apuestas(n_caballos, compartida) == -1){
			printf("Error inicizalizando apuestas\n");
			shmdt((struct _Compartida*)compartida);
			shmctl(id_zone,IPC_RMID,(struct shmid_ds*)NULL);
			free(mensaje);
			free(pids);
			for(int i = 0; i < n_apostadores; i++){
				free(apostadores[i]);
			}
			free(apostadores);
			Borrar_Semaforo(semid);
			exit(EXIT_FAILURE);
		}
		Up_Semaforo(semid,0,SEM_UNDO);

		estructura_hilo->n_caballos = n_caballos;
		estructura_hilo->id_zone = id_zone;
		estructura_hilo->id_mensajes = id_mensajes;

		/*Creamos tantos hilos como ventanillas*/

		for(i = 0; i < n_ventanillas; i++){
			estructura_hilo->id = i +1;
			pthread_create(&hilo[i], NULL, ventanilla, (void*)estructura_hilo);
			Down_Semaforo(semid, 2, SEM_UNDO);
		}

		pause();

		estado = EN_PROCESO;

		for(i = 0; i < n_ventanillas; i++){
			pthread_join(hilo[i], NULL);
			free(estructura_hilo);
		}

		exit(EXIT_SUCCESS);

	}

	/*CREAMOS EL PROCESO APOSTADOR*/
	pid_apostador = fork();
	if(pid_apostador == 0){
		exit(EXIT_SUCCESS);
	}



	/*CREAMOS LOS CABALLOS*/

	for(i = 0 ; i < n_caballos; i++){
			/*Creamos tantos hijos como caballos*/
		pids[i] = fork();

		if(pids[i] < 0){
/*FALTAN CONTROLES DE ERORRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR*/
			printf("Error creando un hijo");
			shmdt((struct _Compartida*)compartida);
			shmctl(id_zone,IPC_RMID,(struct shmid_ds*)NULL);
			free(mensaje);
			free(pids);
			for(int i = 0; i < n_apostadores; i++){
				free(apostadores[i]);
			}
			free(apostadores);
			Borrar_Semaforo(semid);
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

	shmdt((struct _Compartida*)compartida);
	shmctl(id_zone,IPC_RMID,(struct shmid_ds*)NULL);

	exit(EXIT_SUCCESS);
}