/**
* @brief Realizacion del ejercicio 9
*  Este modulo contiene la implementacion de la biblioteca para los semaforos definida en el ejercicio9.h
* @file ejercicio9.c
* @author Alberto Gonzalez (alberto.gonzalezk@estudiante.uam.es) y Sofia sanchez (sofia.sanchezf@estudiante.uam.es)
* @version 1.0
* @date
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <signal.h>
#include "ejercicio8.h"

#define MAX_CAJAS 4
#define MAX_NUM 50
#define FILEKEY "/bin/cat" 
#define KEY1 1301
#define HIJO_FILE "hijo.txt"
#define CUENTA_GLOBAL "cuentaGlobal.txt"
#define MAX_TAM 512

/**
 * @brief Generador de numeros aleatorios entre un infimo y un supremo
 *
 * @param inf Infimo
 * @param sup supremo
 *
 * @return numero aleatorio
 */
int aleat_num(int inf, int sup);
void captura (int sennal);


/**
* @brief main del ejercicio 9
* Este es el programa que implementa un sistema de gestion de transcciones bancarias
*/
int main(){
	char ficherosCliente[MAX_CAJAS][MAX_TAM];
	char ficherosCaja[MAX_CAJAS][MAX_TAM];
	int i, j, k, childpid[MAX_CAJAS], cantidad, total, check, semid1, semid2, aux, num;
	unsigned short array1[MAX_CAJAS];
	unsigned short array2 = 1;
	int hijos = 0;
	char finish = 'F';
	char buff;
	sigset_t set;
	FILE *pf;

	/*Escribimos en el archivo de la cuenta que lleva el padre un 0 para poder realizar operaciones más adelante*/
	for(i = 0; i < MAX_CAJAS; i++){
		array1[i] = 1;
	}
	pf = fopen(CUENTA_GLOBAL, "w");
	if(!pf){
		perror("fopen inicial");
		exit(ERROR);
	}
	fprintf(pf, "0");
	fclose(pf);

	if (signal(SIGUSR1, captura) == SIG_ERR){
   		perror("Error en la captura.\n");
      	exit(EXIT_FAILURE);
   }
	/*Creamos una mascara que tenga solo la señal USR1 desbloqueada, para que asi el padre pueda esperarla*/
	if (sigfillset(&set) == -1) {
		perror("Error en sigfillset.\n");
		exit(ERROR);
	}
	check = sigdelset(&set, SIGUSR1);
	if(check == ERROR){
		perror("sigdelset");
		exit(ERROR);
	}

	sigdelset(&set, SIGINT);

	/*Creamos un semaforo de tamaño del numero de hijos a tener para controlar que el padre y los hijos no accedan a las cajas a la vez*/
	check = Crear_Semaforo(IPC_PRIVATE, MAX_CAJAS, &semid1);
	if(check == ERROR){
		perror("crear semaforo 1");
		exit(ERROR);
	}

	/*Inicializamos el semaforo a 1*/
	check = Inicializar_Semaforo(semid1, array1);
	if(check == ERROR){
		Borrar_Semaforo(semid1);
		perror("inicializar semaforo 1");
		exit(ERROR);
	}

	/*Creamos un semaforo de tamaño uno para controlar que los hijos y el padre no accedan al archivo HIJO_FILE a la vez*/
	check = Crear_Semaforo(IPC_PRIVATE, 1, &semid2);
	if(check == ERROR){
		Borrar_Semaforo(semid1);
		Borrar_Semaforo(semid2);
		perror("crear semaforo 2");
		exit(ERROR);
	}
	/*Inicializamos el semaforo a 1*/
	check = Inicializar_Semaforo(semid2, &array2);
	if(check == ERROR){
		Borrar_Semaforo(semid1); 
		Borrar_Semaforo(semid2);
		perror("inicializar semaforo 2");
	}
	/*Guardamos el nombre de los archivos de los clientes en un array para que cada hijo acceda al que le corresponda*/
	for(i = 0; i < MAX_CAJAS; i++){
      /*guardamos los nombres de los ficheros*/
		sprintf(ficherosCliente[i], "clientesCaja%d.txt", i+1);
		pf = fopen(ficherosCliente[i], "w");
		if(!pf){
			Borrar_Semaforo(semid1);
			Borrar_Semaforo(semid2);
			perror("fopen");
			exit(ERROR);
		}
		/*Introducimos MAX_NUM numeros aleatorios entre 0 y 300 en el archivo*/
		for(j = 0; j < MAX_NUM; j++){
			aux = aleat_num(0, 300);
			fprintf(pf, "%d\n", aux);
		}
		fclose(pf);
	}
	/*Guardamos el nombre de los archivos de las cajas en un array para que cada hijo acceda al que le corresponda*/
	for(i = 0; i < MAX_CAJAS; i++){
		sprintf(ficherosCaja[i], "caja%d.txt", i+1);
		pf = fopen(ficherosCaja[i], "w");
		fprintf(pf, "0");
		fclose(pf);

	}
	/*El padre crea MAX_CAJAS hijos*/
	for (i = 0; i < MAX_CAJAS; i++){
		childpid[i] = fork();
      /* si hay un error en el fork, liberamos la memoria reservada anteriormente*/
		if(childpid[i] < 0){
			Borrar_Semaforo(semid1);
			Borrar_Semaforo(semid2);
			perror("fork");
			exit(ERROR);
		}
		else if(childpid[i] == 0){
			break;
		}
	}
	/*Codigo que ejecutan los hijos*/
	if(childpid[i] == 0 && i != MAX_CAJAS){

		for(j = 0; j < MAX_NUM; j++){
			pf = fopen(ficherosCliente[i], "r"); /*lo abrimos en modo lectura*/
			if(!pf){
				Borrar_Semaforo(semid1); 
				Borrar_Semaforo(semid2);
				perror("fopen cliente hijo");
				exit(ERROR);
			}
			/*Lee el valor que le corresponda*/
			for(k = 0; k<=j; k++){
				fscanf(pf, "%d", &cantidad);
			}
			fclose(pf);
			/*Duerme*/
			sleep(aleat_num(1, 1));
			/*Protejo el acceso al archivo de la caja*/
			check = Down_Semaforo(semid1, i, SEM_UNDO);
			if(check == ERROR){
				Borrar_Semaforo(semid1); 
				Borrar_Semaforo(semid2);
				perror("Down hijo semid1");
				exit(ERROR);
			}
			pf = fopen(ficherosCaja[i], "r");
			/*Si no es la primera iteracion leemos lo que hay en la caja y lo sumamos a la nueva cantidad. 
			Si es la primera escribimos directamente cantidad*/
			fscanf(pf, "%d", &total);
			total += cantidad;
			fclose(pf);
			pf = fopen(ficherosCaja[i], "w");
			if(!pf){
				 printf("jeje ");
				fflush(stdout);
			}
			fprintf(pf, "%d", total);
			fclose(pf);
			/*Si el total es 1000 o mas hay que avisar al padre. En la ultima iteracion no se hace puesto que se vacia la caja*/
			if(total >= 1000 || j == MAX_NUM - 1){
				/*Protegemos el acceso a HIJO_FILE para que no entren varios hijos a la vez*/
				check = Down_Semaforo(semid2, 0, SEM_UNDO);
				if(check == ERROR){
					Borrar_Semaforo(semid1); 
					Borrar_Semaforo(semid2);
					perror("Down hijo semid2");
					exit(ERROR);
				}
				pf = fopen(HIJO_FILE, "w");
				if(!pf){
					Borrar_Semaforo(semid1); 
					Borrar_Semaforo(semid2);
					perror("fopen hijo");
					exit(ERROR);
				}
				/*Escribimos que hijo es para que el padre sepa a que archivo acceder*/
				fprintf(pf,"%d ", i);
				/*Si es la ultima iteracion indicamos al padre que hemos terminado*/
				if(j == MAX_NUM - 1){
					fprintf(pf, "%c", finish);
				}
				fclose(pf);
				/*Mandamos la señal al padre*/
				kill(getppid(), SIGUSR1);
			}

			/*Desbloqueamos el semaforo para que el padre pueda acceder a la caja*/
			check = Up_Semaforo(semid1, i, SEM_UNDO);
			if(check == ERROR){
				Borrar_Semaforo(semid1); 
				Borrar_Semaforo(semid2);
				perror("Up hijo");
				exit(ERROR);
			}

			fflush(stdout);
		}
		
	}

	/*Codigo que ejecuta el padre*/
	else{
		while(hijos < MAX_CAJAS){
			/*Espera a la señal USR1*/
			sigsuspend(&set);
			/*Lee que hijo el ha enviado la señal y si ha terminado*/
			pf = fopen(HIJO_FILE, "r");
			if(!pf){
				Borrar_Semaforo(semid1); 
				Borrar_Semaforo(semid2);
				perror("fopen padre");
				exit(ERROR);
			}
			/*Guardamos el numero de elementos leidos para saber si el hijo ha terminado*/
			num = fscanf(pf, "%d %c", &i, &buff);
			fclose(pf);
			/*Protegemos el acceso a la caja*/
			check = Down_Semaforo(semid1, i, SEM_UNDO);
			if(check == ERROR){
				Borrar_Semaforo(semid1); 
				Borrar_Semaforo(semid2);
				perror("Down padre");
				exit(ERROR);
			}
			pf = fopen(ficherosCaja[i], "r");
			if(!pf){
				Borrar_Semaforo(semid1); 
				Borrar_Semaforo(semid2);
				perror("fopen padre");
				exit(ERROR);
			}
			fscanf(pf, "%d", &total);
			fclose(pf);
			pf = fopen(ficherosCaja[i], "w");
			if(!pf){
				Borrar_Semaforo(semid1); 
				Borrar_Semaforo(semid2);
				perror("fopen padre");
				exit(ERROR);
			}
			/*Si el hijo ha terminado se quita todo el dinero. Sino, solo 900*/
			if(num == 2 && buff == finish){
				cantidad = total;
				total = 0;
				fprintf(pf, "%d", total);
				hijos++;
			}
			else{
				cantidad = 900;
				total = total - cantidad;
				fprintf(pf, "%d", total);
			}
			fclose(pf);
			/*Desbloqueamos el acceso a la caja*/
			check = Up_Semaforo(semid1, i, SEM_UNDO);
			if(check == ERROR){
				Borrar_Semaforo(semid1); 
				Borrar_Semaforo(semid2);
				perror("Up padre");
				exit(ERROR);
			}	
			/*Introducimos el dinero obtenido en la cuenta global*/
			pf = fopen(CUENTA_GLOBAL, "r");
			if(!pf){
				Borrar_Semaforo(semid1); 
				Borrar_Semaforo(semid2);
				perror("fopen padre");
				exit(ERROR);
			}
			fscanf(pf, "%d", &total);
			fclose(pf);
			total+=cantidad;
			pf = fopen(CUENTA_GLOBAL, "w");
			if(!pf){
				Borrar_Semaforo(semid1); 
				Borrar_Semaforo(semid2);
				perror("fopen padre");
				exit(ERROR);
			}
			fprintf(pf, "%d", total);
			fclose(pf);
			/*Desbloqueamos al acceso al fichero HIJO_FILE para que el siguiente hijo pueda enviar la señal al padre*/
			check = Up_Semaforo(semid2, 0, SEM_UNDO);
			if(check == ERROR){
				Borrar_Semaforo(semid1); 
				Borrar_Semaforo(semid2);
				perror("Up padre");
				exit(ERROR);
			}
		}
		/*Cuando todos los hijos han terminado borramos los semaforos*/
		Borrar_Semaforo(semid1); 
		Borrar_Semaforo(semid2);	
	}

	exit(EXIT_SUCCESS);
}

int aleat_num(int inf, int sup){
	
	if ((inf<0)||(sup<0)) return ERROR;
	if(inf > sup ) return ERROR;
	if (inf == sup) return inf;

	return inf + rand() % (sup-inf+1);
}
void captura(int sennal){
   return;
}
