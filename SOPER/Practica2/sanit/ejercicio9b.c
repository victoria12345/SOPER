#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/shm.h>
#include <stdlib.h>
#include "semaforos.h"

#define CAJAS 4
#define SEMKEY 75798
#define PALABRA 100
#define FILEKEY "/bin/cat"
#define CUENTA_GLOBAL "cuentaGlobal.txt"
#define FICH_HIJO "hijo.txt"

int aleatorio(int inf, int supremo){
	if(inf < 0|| supremo <0 || supremo < inf){
		return -1;
	}

	return inf + rand() % (supremo-inf+1);
}

void manejador_SIGUSR1(int sig){

	signal(sig, SIG_IGN);

	signal(sig,manejador_SIGUSR1);

}



int main(void){
	int sem_id1, sem_id2,i, childpid[CAJAS];
	int k, cantidad, total;
	unsigned short array[CAJAS];
	char clientes[CAJAS][PALABRA];
	char cajas[CAJAS][PALABRA];
	FILE* pf;
	sigset_t set, aux;

	for(i = 0; i < CAJAS; i++){
		array[i] = 1;
	}

	pf = fopen(CUENTA_GLOBAL, "w");
	if(!pf){
		return -1;
	}
	/*Dinero inicial en la cuenta*/
	fprintf(pf, "0" );
	fclose(pf);

	/*Creamos mascara que solo tenga desbloqueada la sennal USR1*/
	sigfillset(&set);
	sigprocmask(SIG_BLOCK, &set, &aux);
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigprocmask(SIG_UNBLOCK, &set, &aux);

	/* Creamos los dos semaforos que vamos a usar*/

	if(Crear_Semaforo(SEMKEY, CAJAS, &sem_id1) == ERROR){
		printf("Error creando el primer semafro\n");
		exit(ERROR);
	}

	if(Crear_Semaforo(SEMKEY, 1, &sem_id2) == ERROR){
		printf("Error creando el segundo semafro");
		Borrar_Semaforo(sem_id1);
		exit(ERROR);
	}

	if( Inicializar_Semaforo(sem_id1, array) == ERROR){
		Borrar_Semaforo(sem_id1);
		Borrar_Semaforo(sem_id2);
		printf("Error al inicializar el semaforo1\n");
		exit(ERROR);
	}

	if( Inicializar_Semaforo(sem_id2, array) == ERROR){
		Borrar_Semaforo(sem_id1);
		printf("Error al inicializar el semaforo2\n");
		exit(ERROR);
	}

	/*Guardamos en un array el nombre de los ficheros*/
	for(i = 0; i < CAJAS; i++){
		int j;
		sprintf(clientes[i], "clientesCaja%d.txt", i+1);
		pf = fopen(clientes[i], "w");
		if(!pf){
			Borrar_Semaforo(sem_id1);
			Borrar_Semaforo(sem_id2);
			printf("Error abriendo fichero %s", clientes[i]);
			exit(ERROR);
		}

		/*Introducimos los 50 num aleatorios en los ficheros*/
		for(j = 0; j < 50; j++){
			int x;
			x = aleatorio(0,300);
			fprintf(pf,"%d\n", x);
		}
		fclose(pf);
	}

	/*Creamos e inicializamos a 0 las cajas*/
	for(i = 0; i < CAJAS; i++){
		sprintf(cajas[i], "caja%d.txt", i);
		pf = fopen(cajas[i], "w");
		if(!pf){
			Borrar_Semaforo(sem_id1);
			Borrar_Semaforo(sem_id2);
		}
		/*Escribimos 0 porque inicialmente estan vacias*/
		fprintf(pf, "0");
		fclose(pf);
	}

	/*El padre crea tantos hijos como cajas*/
	for(i = 0; i < CAJAS; i++){
		childpid[i] = fork();

		if(childpid[i] < 0){
			Borrar_Semaforo(sem_id1);
			Borrar_Semaforo(sem_id2);
			fclose(pf);
			printf("Error creando el hijo\n");
			exit(ERROR);
		}else if(childpid[i] == 0){
			break;
		}
	}

	if(childpid[i] == 0 && i != CAJAS){
		int j;
		for(j = 0; j < 50; j++){

			pf = fopen(clientes[i], "r");
			if(!pf){
				Borrar_Semaforo(sem_id1);
				Borrar_Semaforo(sem_id2);
				printf("Error leyendo fichero cliente %d", i);
				exit(ERROR);
			}
			/*j es el n-esimo numero que tiene que leer*/
			for(k = 0; k < j; k++){
				fscanf(pf,"%d", &cantidad);
			}
			sleep(aleatorio(1,5));
			fclose(pf);
		}

			/*Abrimos la caja*/
			if(Down_Semaforo(sem_id1, i, SEM_UNDO) == ERROR){
				Borrar_Semaforo(sem_id1);
				Borrar_Semaforo(sem_id2);
				printf("Error bloqueando el semaforo para abrir caja \n");
				exit(ERROR);
			}

			pf = fopen(cajas[i], "r");
			if(!pf){
				Borrar_Semaforo(sem_id1);
				Borrar_Semaforo(sem_id2);
				printf("Error error abriendo caja %d", i);
				exit(ERROR);
			}

			fscanf(pf,"%d", &total);
			total += cantidad;
			fclose(pf);
			/*Escribimos el total que llevmaos*/
			pf = fopen(cajas[i], "w");
			if(!pf){
				Borrar_Semaforo(sem_id1);
				Borrar_Semaforo(sem_id2);
				printf("Error error abriendo caja %d", i);
				exit(ERROR);
			}
			fprintf(pf,"%d", total);
			fclose(pf);

			/*Comprobamos que el total sea menor o igual a 1000*/
			/*o que este leyendo el ultimo numero*/
			if(total >= 1000 || j == 49){
				/*Protegemos el acceso al archivo de hijos*/
				if(Down_Semaforo(sem_id2,0,SEM_UNDO) == ERROR){
					Borrar_Semaforo(sem_id1);
					Borrar_Semaforo(sem_id2);
					printf("Error abriendo el archivo de los hijos\n");
					exit(ERROR);
				}
				/*Escribimos que hijo es para que sepa que caja vaciar*/
				pf = fopen(FICH_HIJO, "w");
				if(!pf){
					Borrar_Semaforo(sem_id1);
					Borrar_Semaforo(sem_id2);
					printf("Error abriendo el archivo de los hijos\n");
					exit(ERROR);
				}
				fprintf(pf, "%d", i);
				if(j == 49){
					/*Si tenia que terminar escribe la sennal de que termina*/
					fprintf(pf, "F");
				}
				fclose(pf);
				/*mandamos sennal al padre para que lo mire*/
				kill(getppid(), SIGUSR1);
			}

			/*Se desbloquea el semaforo para que el apdre pueda entrar a la caja*/
			if(Up_Semaforo(sem_id1, i, SEM_UNDO) == ERROR){
				Borrar_Semaforo(sem_id1);
				Borrar_Semaforo(sem_id2);
				printf("Error desbloqueando la caja de los hijos\n");
				exit(ERROR);
			}

			fflush(stdout);

		}
		/* PADRE*/
		else{
			int terminados = 0;
			while(terminados < CAJAS){
				int n;
				char c;
				void manejador_SIGUSR1();
				signal(SIGUSR1, manejador_SIGUSR1); /*Armar la señal */

				pause();

				/*Cuando el padre recibe la sennal*/
				pf = fopen(FICH_HIJO, "r");
				if(!pf){
					Borrar_Semaforo(sem_id1);
					Borrar_Semaforo(sem_id2);
					printf("Error abriendo fich hijo");
					exit(ERROR);
				}

				n = fscanf(pf, "%d %c", &i, &c);
				fclose(pf);



				/*Protegemos la caja*/
				if(Down_Semaforo(sem_id1, i, SEM_UNDO) == ERROR){
					Borrar_Semaforo(sem_id1);
					Borrar_Semaforo(sem_id2);
					printf("Error protegiendo la caja");
					exit(ERROR);
				}

				pf = fopen(cajas[i], "r");
				if(!pf){
					Borrar_Semaforo(sem_id1);
					Borrar_Semaforo(sem_id2);
					printf("Error abriendo la caja");
					exit(ERROR);
				}

				fscanf(pf,"%d", &total);
				fclose(pf);
				pf = fopen(cajas[i], "w");
				if(!pf){
					Borrar_Semaforo(sem_id1);
					Borrar_Semaforo(sem_id2);
					printf("Error abriendo la caja");
					exit(ERROR);
				}

				if(n == 2 && c == 'F'){
					cantidad = total;
					terminados++;
					fprintf(pf, "0");
				}
				fclose(pf);

				/*Desbloquea la caja*/
				if(Up_Semaforo(sem_id1, i, SEM_UNDO) == ERROR){
					Borrar_Semaforo(sem_id1);
					Borrar_Semaforo(sem_id2);
					printf("Error desbloqueando una caja");
					exit(ERROR);
				}

				pf = fopen(CUENTA_GLOBAL, "r");
				if(!pf){
					Borrar_Semaforo(sem_id1);
					Borrar_Semaforo(sem_id2);
					printf("Error dabriendo cuenta global");
					exit(ERROR);
				}
				fscanf(pf, "%d", total);
				fclose(pf);
				total += cantidad;

				pf = fopen(CUENTA_GLOBAL, "W");
				if(!pf){
					Borrar_Semaforo(sem_id1);
					Borrar_Semaforo(sem_id2);
					printf("Error dabriendo cuenta global");
					exit(ERROR);
				}
				fprintf(pf, "%d", total);
				fclose(pf);

				/*Desbloqueamos la cuenta toal*/
				if(Up_Semaforo(sem_id2, 0, SEM_UNDO) == -1){
					Borrar_Semaforo(sem_id1);
					Borrar_Semaforo(sem_id2);
					printf("Error bloqueando cuenta global");
					exit(ERROR);
				}
			}
			Borrar_Semaforo(sem_id1);
			Borrar_Semaforo(sem_id2);
		}

		return 0;

}
