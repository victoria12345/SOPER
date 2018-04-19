/**
* @brief Realizacion del ejercicio 5
* Programa que implementa una cadena de montaje utilizando 3 procesos y una cola de mensajes
* El proceso A trocea el mensaje proveniente del fichero y lo introduce en la cola de mensajes
* El proceso B modifica los mensajes y los envia de nuevo a la cola
* El proceso C vuelva el contenido de la cola de mensajes en el fichero de salida
* @file cadena_montaje.c
* @author Ignacio Rabuñal García y Victoria Pelayo Alvaredo
* @version 1.0
* @date
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/stat.h>
#define FILEKEY "/bin/ls"
#define KEY 1234
#define MAXCHAR 2000

typedef struct _Mensaje{
  long id; /*identificador del mensaje*/
  char texto[MAXCHAR + 1];/*texto del mensaje*/
}mensaje;

/**
* @brief modifica un texto, convirtiendo cada letra en la siguiente en el abecedario y la 'z' en la 'a'
* @param texto texto a modificar
*/
void convertir_letras(char texto[MAXCHAR +1]);

/**
* @brief Realizacion del ejercicio 5
* Programa que implementa una cadena de montaje utilizando 3 procesos y una cola de mensajes
* El proceso A trocea el mensaje proveniente del fichero y lo introduce en la cola de mensajes
* El proceso B modifica los mensajes y los envia de nuevo a la cola
* El proceso C vuelva el contenido de la cola de mensajes en el fichero de salida
* @param f1 fichero de entrada que contiene el mensaje con el que el programa va a trabajar
* @param f2 nombre del fichero de salida en el que se volcara el mensaje modificado
* @author Ignacio Rabuñal García y Victoria Pelayo Alvaredo
* @version 1.0
* @date
*/
int main(int argc, char const *argv[]){
	int i, j, msqid, forigen, fdestino, nmensajes, pid;
	int *status = NULL;
	key_t key;
	mensaje mensaje;
	struct stat buf;

	if (argc < 3){
		fprintf(stderr, "Introduzca dos ficheros al ejecutar\n");
		exit(EXIT_FAILURE);
	}

	key = ftok(FILEKEY, KEY);
	if(key == -1) {
		fprintf (stderr, "Error con la clave \n");
		return -1; 
	}

	msqid = msgget (key, 0600 | IPC_CREAT);
	if (msqid == -1){
		perror("Error al obtener identificador para cola mensajes \n");
		exit(EXIT_FAILURE);
	}

	forigen = open(argv[1], O_RDONLY, 0x0777);
	if(forigen == -1){
		fprintf(stderr, "Error al abrir el fichero\n");
		msgctl(msqid, IPC_RMID, (struct msqid_ds*)NULL);
		exit(EXIT_FAILURE);
	}

	if(fstat(forigen, &buf) == -1){
		fprintf(stderr, "Error al extraer tamaño del fichero\n");
		msgctl(msqid, IPC_RMID, (struct msqid_ds*)NULL);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "Comienza el envio de mensajes\n");

	if(buf.st_size < MAXCHAR){
		nmensajes = 1;
	}else{
		nmensajes = ceil(buf.st_size/MAXCHAR);
	}

	for (i = 0; i < nmensajes; ++i){
		mensaje.id = 1;
		pread(forigen, (void*)&mensaje.texto, MAXCHAR, MAXCHAR*i);
		msgsnd(msqid, (struct msgbuf*)&mensaje, sizeof(mensaje) - sizeof(long), IPC_NOWAIT);
	}

	close(forigen);

	for(i = 0; i < 2; i++){
		pid = fork();

		if(pid < 0){
			fprintf(stderr, "Error al crear hijos\n");
			msgctl(msqid, IPC_RMID, (struct msqid_ds*)NULL);
			exit(EXIT_FAILURE);
		}

		if(pid == 0){
			if(i == 0){
				/*Proceso que modifica los mensajes. Cambia el id de los mensajes para que no sean confundidos con los iniciales*/
				msqid = msgget (key, 0600 | IPC_CREAT);
				if (msqid == -1){
					perror("Error al obtener identificador para cola mensajes \n");
					exit(EXIT_FAILURE);
				}

				fprintf(stderr, "Modificando mensajes y enviandolos de nuevo\n");
				for(j = 0; j < nmensajes; j++){
					msgrcv(msqid, (struct msqid_ds*)&mensaje, sizeof(mensaje) - sizeof(long), 1, 0);
					convertir_letras(mensaje.texto);
					mensaje.id = 2;
					msgsnd(msqid, (struct msgbuf*)&mensaje, sizeof(mensaje) - sizeof(long), IPC_NOWAIT);
				}

				exit(EXIT_SUCCESS);
			}else if(i == 1){
				/*Proceso que vuelca el contenido de los mensajes en el fichero*/
				fdestino = open(argv[2], O_WRONLY | O_APPEND | O_CREAT, 0x0777);
				if(fdestino == -1){
					fprintf(stderr, "Error al abrir el fichero\n");
					exit(EXIT_FAILURE);
				}

				msqid = msgget(key, 0600 | IPC_CREAT);
				if (msqid == -1){
					close(fdestino);
					exit(EXIT_FAILURE);
				}

				fprintf(stdout, "Escribiendo los nuevos mensajes en el fichero de destino\n");
				for(j = 0; j < nmensajes; j++){
					msgrcv(msqid, (struct msqid_ds*)&mensaje, sizeof(mensaje) - sizeof(long), 2, 0);
					write(fdestino, (void*)&mensaje.texto, strlen(mensaje.texto));
				}

				close(fdestino);
				exit(EXIT_SUCCESS);
			}else{
				exit(EXIT_FAILURE);
			}
		}else{
			waitpid(pid, status, WUNTRACED | WCONTINUED);
		}
	}

	msgctl(msqid, IPC_RMID, (struct msqid_ds*)NULL);
	exit(EXIT_SUCCESS);
	return 0;
}

void convertir_letras(char texto[MAXCHAR +1]){
	int i;

	if(texto == NULL){
		return;
	}

	for(i = 0; i < MAXCHAR; i++){
		if(texto[i] == '\0'){
			return;
		}else if(texto[i] == 'z'){
			texto[i] = 'a';
		}else{
			texto[i]++;
		}

	}
}
