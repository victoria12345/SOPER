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
#include <pthread.h>
#include "caballos.h"
#include "semaforos.h"
#include "apuestas.h"
#include "monitor.h"

#define FILEKEY "/bin/cat"

#define KEY 1347
#define SEMKEY 1377
#define KEY2 21376

#define MAX_WORD 200

#define ACABADA 4
#define EN_PROCESO 5
#define ANTES 6

int estado;
unsigned short *array;
int n_semaforos = 4;
int semid;

void* ventanilla(void* estructura_hilo){
	char *tmp;
	int id;
	Apuesta apuesta;
	Compartida* compartida;

	FILE *f;

	if(estructura_hilo == NULL){
		pthread_exit(NULL);
	}

	id = ((Estructura_hilo*)estructura_hilo)->id;

	Up_Semaforo(semid,2,SEM_UNDO);

	compartida = (Compartida*)malloc(sizeof(Compartida));
	compartida = shmat(((Estructura_hilo*)estructura_hilo)->id_zone, (struct Compartida*)0,0);
	if(compartida == NULL){
		printf("Error reservando memoria para compartida en ventanilla\n");
		exit(EXIT_FAILURE);
	}

	while(estado == ANTES){

		if(msgrcv(((Estructura_hilo*)estructura_hilo)->id_mensajes , (struct msgbuf*)&apuesta, sizeof(apuesta)-sizeof(long), 1, IPC_NOWAIT)>0){

			if(apuesta.caballo > ((Estructura_hilo*)estructura_hilo)->n_caballos|| apuesta.caballo <0){
				printf("Numero de caballo no valido\n");
				pthread_exit(NULL);
			}

			if((tmp = (char* )malloc(sizeof(char) * MAX_WORD)) == NULL){
				printf("Error con reserva memoria tmp en ventanilla\n");
				pthread_exit(NULL);
			}
			Down_Semaforo(semid, 1, SEM_UNDO);
			if((f = fopen("apuestas.txt", "a")) == NULL){
				printf("Error abriendo fichero de apuestas\n");
				free(tmp);
				pthread_exit(NULL);
			}

			sprintf(tmp, "%s Ventanilla: %d Caballo: %d Apuesta: %f Cotizacion: %f \n", apuesta.apostador, id,apuesta.caballo, apuesta.apuesta,compartida->cotizacion[apuesta.caballo]);
			fwrite(tmp, strlen(tmp), 1, f);
			free(tmp);

			fclose(f);
			Up_Semaforo(semid, 1, SEM_UNDO);
			Down_Semaforo(semid, 0, SEM_UNDO);

			compartida->apuestas[apuesta.caballo] += apuesta.apuesta;
			compartida->total_apuestas += apuesta.apuesta;
			compartida->cotizacion[apuesta.caballo] = compartida->total_apuestas / compartida->apuestas[apuesta.caballo];
			Up_Semaforo(semid, 0, SEM_UNDO);
		}
	}

	pthread_exit(NULL);
}


int main(int argc, char const *argv[]){


	int n_caballos, longitud, n_apostadores, n_ventanillas, cantidad;
	int i, j;

	key_t clave;
	int id_mensajes;

	int pid_apostador, pid_gestor, pid_monitor;

	char *mensaje;

	char** apostadores;

	int *pids, *posicion;

	int pipes[MAX_CABALLOS][2];

	pthread_t* hilo;
	Estructura_hilo* estructura_hilo;

	/*Para memoria compartida*/
	Compartida* compartida;
	key_t clave_mem, clave_mem2;

	int id_zone, id_zone2;

	Mensaje msj;

	estado = ANTES;

	FILE *f;

	f = fopen("apuestas.txt", "w");
	if(f == NULL){
		printf("Error al iniciar el fichero \n");
		exit(EXIT_FAILURE);
	}
	fclose(f);

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
				for( i = 0; i < n_apostadores; i++){
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
			for( i = 0; i < n_apostadores; i++){
				free(apostadores[i]);
			}
			free(apostadores);
			Borrar_Semaforo(semid);
			exit(EXIT_FAILURE);
		}

		array[0] = 1;
		array[1] = 1;
		array[2] = 0;
		array[3] = 1;

		if(Inicializar_Semaforo(semid,array) == ERROR){
			printf("Error al incializar el semaforo\n");
			free(mensaje);
			for( i = 0; i < n_apostadores; i++){
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
			for( i = 0; i < n_apostadores; i++){
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
			for( i = 0; i < n_apostadores; i++){
				free(apostadores[i]);
			}
			free(apostadores);
			free(posicion);
			Borrar_Semaforo(semid);
			exit(EXIT_FAILURE);
		}

		for(i = 0; i < n_caballos; i++){
			posicion[i] = 0;
		}

		/*Hilos*/
		hilo = (pthread_t *)malloc(sizeof(pthread_t) * n_ventanillas);
		if(hilo == NULL){
			printf("Error reservando memoria para los hilos\n");
			printf("Error con id_zone\n");
			free(mensaje);
			free(pids);
			for( i = 0; i < n_apostadores; i++){
				free(apostadores[i]);
			}
			free(apostadores);
			free(posicion);
			Borrar_Semaforo(semid);
			exit(EXIT_FAILURE);
		}

		/*Memoria compartda*/
		if((clave_mem = ftok(FILEKEY, KEY)) == -1){
			printf("Error con la clave memoria compartida\n");
			free(mensaje);
			free(pids);
			for( i = 0; i < n_apostadores; i++){
				free(apostadores[i]);
			}
			free(apostadores);
			free(posicion);
			Borrar_Semaforo(semid);
			exit(EXIT_FAILURE);
		}

		id_zone = shmget(clave_mem, sizeof(struct _Compartida), IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
		if(id_zone == -1){
			printf("Error con id_zone\n");
			free(mensaje);
			free(pids);
			for( i = 0; i < n_apostadores; i++){
				free(apostadores[i]);
			}
			free(apostadores);
			free(posicion);
			free(hilo);
			Borrar_Semaforo(semid);
			exit(EXIT_FAILURE);
		}


		compartida = shmat(id_zone, (struct _Compartida*)0,0);
		if(compartida == NULL){
			printf("Error con compartida al reservar\n");
			free(mensaje);
			free(pids);
			for( i = 0; i < n_apostadores; i++){
				free(apostadores[i]);
			}
			free(apostadores);
			free(posicion);
			free(hilo);
			Borrar_Semaforo(semid);
			exit(EXIT_FAILURE);
		}

		/*En este momento no es necesario utilizar semaforos*/
		if(Inicializar_apuestas(n_caballos, compartida) == -1){
			printf("Error inicializando las apuestas\n");
			free(mensaje);
			free(pids);
			for( i = 0; i < n_apostadores; i++){
				free(apostadores[i]);
			}
			free(apostadores);
			free(posicion);
			free(hilo);
			Borrar_Semaforo(semid);
			exit(EXIT_FAILURE);
		}
/*PROBANDOOOOO*/

		if((clave_mem2= ftok(FILEKEY, KEY2)) == -1){
			printf("Error con la clave memoria compartida2\n");
			free(mensaje);
			free(pids);
			for( i = 0; i < n_apostadores; i++){
				free(apostadores[i]);
			}
			free(apostadores);
			free(posicion);
			Borrar_Semaforo(semid);
			exit(EXIT_FAILURE);
		}
		id_zone2 = shmget(clave_mem2, sizeof(struct _Compartida_monitor), IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
		if(id_zone2 == -1){
			printf("Error con id_zone2\n");
			free(mensaje);
			free(pids);
			for( i = 0; i < n_apostadores; i++){
				free(apostadores[i]);
			}
			free(apostadores);
			free(posicion);
			free(hilo);
			Borrar_Semaforo(semid);
			exit(EXIT_FAILURE);
		}

		estructura_hilo = (Estructura_hilo*)malloc(sizeof(Estructura_hilo));

		/*CREAMOS PROCESO MONITOR*/

		pid_monitor = fork();
		if(pid_monitor < 0){
			printf("error creando monitor\n");
			free(mensaje);
			free(pids);
			for( i = 0; i < n_apostadores; i++){
				free(apostadores[i]);
			}
			free(apostadores);
			free(posicion);
			free(hilo);
			Borrar_Semaforo(semid);
			exit(EXIT_FAILURE);
		}else if(pid_monitor == 0){

			if(signal(SIGUSR1, manejador_carrera) == SIG_ERR){
				printf("Error con manejador carrera en monitor\n");
				exit(EXIT_FAILURE);
			}

			pause();

			Iniciar_monitor(semid, n_caballos, compartida);

			is(estado == EN_PROCESO){
				Compartida_monitor* compartida;
				int i;

				compartida = (Compartida_monitor*)malloc(sizeof(Compartida_monitor));

				compartida = shmat(id_zone2, (struct _Compartida_monitor*)0,0);
				if(compartida == NULL){
					printf("Error con compartida al reservar\n");
					free(mensaje);
					free(pids);
					for( i = 0; i < n_apostadores; i++){
						free(apostadores[i]);
					}
					free(apostadores);
					free(posicion);
					free(hilo);
					Borrar_Semaforo(semid);
					exit(EXIT_FAILURE);
				}

				while(estado == EN_PROCESO){
					pause();
					Down_Semaforo(semid, 3,SEM_UNDO);

					for(i = 0; i < n_caballos; i++){
						printf("Ultima tirada caballo %d: %d. Posicion: %d\n", i+1, compartida->tiradas[i], posicion[i]);
					}

					Up_Semaforo(semid,3,SEM_UNDO);
				}
				
			}


			exit(EXIT_SUCCESS);
		}



		/*CREAMOS GESTOR DE APUESTAS*/

		pid_gestor = fork();
		if(pid_gestor < 0){
			printf("error creando gestor de apuestas\n");
			free(mensaje);
			free(pids);
			for( i = 0; i < n_apostadores; i++){
				free(apostadores[i]);
			}
			free(apostadores);
			free(posicion);
			free(hilo);
			Borrar_Semaforo(semid);
			exit(EXIT_FAILURE);
		}else if(pid_gestor == 0){
			if(signal(SIGUSR1, manejador_carrera) == SIG_ERR){
				printf("Error con manejador carrera en gestor\n");
				exit(EXIT_FAILURE);
			}
			if(Crear_Semaforo(SEMKEY, n_semaforos, &semid) == ERROR){
				printf("Error creando los semaforos\n");
				free(mensaje);
				free(apostadores);
				exit(EXIT_FAILURE);
			}

			Inicializar_Gestor(estructura_hilo, semid, 0, n_caballos, id_zone, id_mensajes, compartida);
			kill(pid_monitor, SIGUSR1);

			for(i = 0; i < n_ventanillas; i++){
				estructura_hilo->id = i+1;
				pthread_create(&hilo[i], NULL, ventanilla, (void*)estructura_hilo);
				Down_Semaforo(semid, 2, SEM_UNDO);
			}

			pause();
			estado = EN_PROCESO;

			for(i = 0; i < n_ventanillas; i++){
				pthread_join(hilo[i], NULL);
			}

			free(estructura_hilo);
			exit(EXIT_SUCCESS);
		}

		/*CREAMOS PROCESO APOSTADOR*/
		pid_apostador = fork();
		if(pid_apostador < 0){
			printf("Error al crear el proceso apostador\n");
			free(mensaje);
			free(pids);
			for( i = 0; i < n_apostadores; i++){
				free(apostadores[i]);
			}
			free(apostadores);
			free(posicion);
			free(hilo);
			Borrar_Semaforo(semid);

			wait(NULL);
			exit(EXIT_FAILURE);
		}else if(pid_apostador == 0){
			srand(getpid());
			if(signal(SIGUSR1, manejador_final) == SIG_ERR){
				printf("Error con el manejador final en apostador\n");
				exit(EXIT_FAILURE);
			}

			/*Cada segundo se envia una apuesta*/
			for(i = 0; i < n_apostadores; i++){
				sleep(1);
				apostador(i+1, n_caballos, cantidad, id_mensajes);
			}
			exit(EXIT_SUCCESS);

		}
		else{

			/*Tiempo previo a la carrera para apostar*/
			sleep(TIEMPO);

			estado = EN_PROCESO;

			kill(pid_apostador, SIGUSR1);
			kill(pid_gestor, SIGUSR1);

			waitpid(pid_apostador, NULL, WUNTRACED|WCONTINUED);
			waitpid(pid_gestor, NULL, WUNTRACED|WCONTINUED);

		}

		/*CREAMOS LOS CABALLOS*/

		sleep(1);
		for(i = 0 ; i < n_caballos; i++){
			/*Creamos tantos hijos como caballos*/
			pids[i] = fork();

			if(pids[i] < 0){
				printf("Error creando un hijo");
				shmdt((struct _Compartida*)compartida);
				shmctl(id_zone,IPC_RMID,(struct shmid_ds*)NULL);
				free(mensaje);
				free(pids);
				for(i = 0; i < n_apostadores; i++){
					free(apostadores[i]);
				}
				free(apostadores);
				Borrar_Semaforo(semid);
				exit(EXIT_FAILURE);
			}
			else if(pids[i] > 0){

				if(signal(SIGUSR1, manejador_carrera) == SIG_ERR){
					printf("Error con el manejador de la carrera\n");
					exit(EXIT_FAILURE);
				}

				/*envia informacion sobre posicion*/
				sprintf(mensaje, "%d", DADO_NORMAL);
				close(pipes[i][0]);
				write(pipes[i][1], mensaje, strlen(mensaje));

				/*Cuando no se han creado todos los caballos*/
				if(i < n_caballos -1){
					continue;
				}

				while(estado != ACABADA){
					/*Actualizamos las posiciones de los caballos*/
					for(j = 0; j < n_caballos; j++){
						/*El id 1 esta ocupado con el gestor de apuestas*/
						msgrcv(id_mensajes, (struct msgbuf*) &msj, sizeof(Mensaje) - sizeof(long), j+2, 0);

						printf("%d\n", msj.tirada);

						posicion[j] += msj.tirada;
						if(posicion[j] >= longitud){
							estado  = ACABADA;
						}

						sprintf(mensaje, "%d", calcular_tirada(posicion, j, n_caballos));
						close(pipes[j][0]);
						write(pipes[j][1], mensaje, sizeof(mensaje));
					}

/*DECLARAR MEMORIA COMPARTIDA E IR METIENDO LAS TIRADAS*/
					for(j = 0; j < n_caballos; j++){
						//kill(pids[j], SIGUSR1);
						//printf("Caballo %d: %d\n", j+1, posicion[j]);
					}
					//printf( "---------------------------\n");
				}
				fflush(stdout);
				/*Cuando un caballo acaba se envia sennal para que todos acaben la carrera*/
				
				for(j = 0; j < n_caballos; j++){
					kill(pids[j], SIGUSR1);
				}
			}
			/*Codigo de los caballos*/
			else if(pids[i] == 0){

				/*Para que al lanzar los dados no todos tengan lo mismo*/
				srand(getpid());

				if(signal(SIGUSR1, manejador_final) == SIG_ERR){
					printf("Error con el manejador de fin de carrera");
					exit(EXIT_FAILURE);
				}

				while(1){

					memset(mensaje, 0, MAX_WORD);
					/*Leemos mensaje de la tuberia y llamamos a la funcion de los caballos*/
					close(pipes[i][1]);
					if(read(pipes[i][0], mensaje, sizeof(mensaje)) == -1){
						printf("Error leyendo el tipo de tirada\n");
						exit(EXIT_FAILURE);
					}

					if(caballo(atoi(mensaje), id_mensajes,i) == -1){
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
		msgctl (id_mensajes, IPC_RMID, (struct msqid_ds *)NULL);
		free(mensaje);
		free(posicion);
		free(pids);
		free(array);
		free(hilo);
		free(estructura_hilo);
		for(i = 0; i < n_apostadores; i++){
			free(apostadores[i]);
		}
		free(apostadores);
		Borrar_Semaforo(semid);
		waitpid(pid_monitor, NULL, WUNTRACED | WCONTINUED);
		exit(EXIT_SUCCESS);
	}
