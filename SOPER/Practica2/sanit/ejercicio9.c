/* PRACTICA 2 EJERCICIO 9 */

/****************************************
*
* @file ejercicio9.c
*
* @author Javier Martinez Rubio
* @author Santiago Valderrabano Zamorano
*
* @date 04/04/2018
*
*****************************************/

/***************************************************************************
*
* @brief
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/shm.h>
#include "semaforo.h"

#define SEMKEY 75798

void handlerPadre(int sennal){
  return;
}

int main(int argc, char const *argv[]) {

  if(argc != 2){
    fprintf(stderr, "Los argumentos son erroneos\n");
    fprintf(stderr, "Introduzca el numero de cajas\n");
    exit(EXIT_FAILURE);
  }

  int n = 0;
  n = atoi(argv[1]);
  FILE *pf[n];
  FILE *pfTotal[n];
  FILE *pfSennales = NULL;

  char cajaHijo[n][50];
  char cajaTotalHijo[n][50];
  char dirSennalHijo[11] = "sennal.txt";

  unsigned short *semArray;
  int semid, numSemaforos = n + 1;

  int sennalHijo, pidHijo;

  int pid, aleatTime, aleatDinero;
  int contadorHijosActivos = n;
  int i, j;
  int cuentaGlobal = 0, cuentaHijo = 0, valor = 0;
  int *status = NULL;
  int *pidHijos;

  pidHijos = (int *) malloc (sizeof(int)*n);


  /* SEMAFOROS */

  if(Crear_Semaforo(SEMKEY, numSemaforos, &semid) == ERROR){
    fprintf(stderr, "Error al crear los semaforos\n");
    exit(EXIT_FAILURE);
  }

  semArray = (unsigned short*) malloc(sizeof(short) * numSemaforos);
  if(semArray == NULL){
    Borrar_Semaforo(semid);
    fprintf(stderr, "Error al crear la array de valores del semaforo\n");
    exit(EXIT_FAILURE);
  }

  for(i = 0; i < numSemaforos; i++){
    semArray[i] = 1;
  }

  if(Inicializar_Semaforo(semid, semArray) == ERROR){
    Borrar_Semaforo(semid);
    free(semArray);
    fprintf(stderr, "Error al inicializar el semaforo\n");
    exit(EXIT_FAILURE);
  }

  /* HANDLERS SEÑALES */

  if(signal(SIGUSR1, handlerPadre) == SIG_ERR){
    fprintf(stderr, "Error en la sennal de retirada de dinero\n");
    exit(EXIT_FAILURE);
  }

  if(signal(SIGUSR2, handlerPadre) == SIG_ERR){
    fprintf(stderr, "Error en la sennal de retirada de dinero\n");
    exit(EXIT_FAILURE);
  }

  /* CREACION DE LOS FICHEROS Y ESCRITURA DE LOS NUMEROS ALEATORIOS EN ELLOS*/

  for(i = 0; i < n; i++){

    sprintf(cajaHijo[i], "clientesCaja%d.txt", i);
    pf[i] = fopen(cajaHijo[i], "w");

    if(pf[i] == NULL){
      fprintf(stderr, "Error al abrir el fichero %s\n", cajaHijo[i]);
      exit(EXIT_FAILURE);
    }
    sprintf(cajaTotalHijo[i], "clientesCajaTotal%d.txt", i);

    for(j = 0; j < 50; j++){
      aleatDinero = rand() % (301);
      fprintf(pf[i], "%d\n", aleatDinero);
    }
    if(fclose(pf[i]) != 0){
      fprintf(stderr, "Error al cerrar el fichero %s\n", cajaHijo[i]);
      exit(EXIT_FAILURE);
    }
  }

  for(i = 0; i < n; i++){

    pid = fork();

    if(pid > 0){

      pidHijos[i] = pid;

      if(i < n - 1){

        continue;
      }

      while(contadorHijosActivos > 0){

        pause();

        pfSennales = fopen(dirSennalHijo, "r");
        if(pfSennales == NULL){
          Borrar_Semaforo(semid);
          free(semArray);
          printf("Error al abrir el archivo de sennales\n");
          exit(EXIT_FAILURE);
        }
        fscanf(pfSennales,"%d %d", &pidHijo, &sennalHijo);
        fclose(pfSennales);

        switch (sennalHijo) {
          case SIGUSR1:
            Down_Semaforo(semid, pidHijo, SEM_UNDO);
            cuentaGlobal += 900;
            Up_Semaforo(semid, pidHijo, SEM_UNDO);
            Up_Semaforo(semid, n, SEM_UNDO);
            break;
          case SIGUSR2:
            contadorHijosActivos--;
            Down_Semaforo(semid, pidHijo, SEM_UNDO);
            pfTotal[pidHijo] = fopen(cajaTotalHijo[pidHijo], "r");
            if(pfTotal[pidHijo] == NULL){
              Borrar_Semaforo(semid);
              free(semArray);
              printf("Error al abrir el archivo %s\n", cajaTotalHijo[pidHijo]);
              exit(EXIT_FAILURE);
            }
            fscanf(pfTotal[pidHijo], "%d", &valor);
            fclose(pfTotal[pidHijo]);

            cuentaGlobal += valor;

            Up_Semaforo(semid, pidHijo, SEM_UNDO);
            Up_Semaforo(semid, n, SEM_UNDO);
            fprintf(stdout, "Quedan %d cajas abiertas\n", contadorHijosActivos);
            break;
        }
      }

      fprintf(stdout, "El total ganado ha sido de %d euros\n", cuentaGlobal);

      for(i = 0; i < n; i++){
        waitpid(pidHijos[i], status, WUNTRACED | WCONTINUED);
      }

    }else if(pid == 0){

      char *aEscribir;
      pf[i] = fopen(cajaHijo[i], "r");
      if(pf[i] == NULL){
        Borrar_Semaforo(semid);
        free(semArray);
        printf("Error al abrir el archivo %s\n", cajaHijo[i]);
        exit(EXIT_FAILURE);
      }

      fprintf(stdout, "Abriendo caja %d\n", i);

      for(j = 0; j < 50; j++){

        Down_Semaforo(semid, i, SEM_UNDO);

        fscanf(pf[i], "%d", &valor);

        cuentaHijo += valor;

        pfTotal[i] = fopen(cajaTotalHijo[i], "w");
        if(pfTotal[i] == NULL){
          Borrar_Semaforo(semid);
          free(semArray);
          printf("Error al abrir el archivo %s\n", cajaTotalHijo[i]);
          exit(EXIT_FAILURE);
        }

        aEscribir = (char *) malloc (3);
        sprintf(aEscribir, "%d", cuentaHijo);
        fwrite(aEscribir, 1, sizeof(aEscribir) ,pfTotal[i]);
        free(aEscribir);
        fclose(pfTotal[i]);

        if(cuentaHijo >= 1000){
          cuentaHijo -= 900;
          Down_Semaforo(semid, n, SEM_UNDO);
          pfSennales = fopen(dirSennalHijo, "w");

          if(pfSennales == NULL){
            Borrar_Semaforo(semid);
            free(semArray);
            printf("Error al abrir el archivo de sennales\n");
            exit(EXIT_FAILURE);
          }
          aEscribir = (char *) malloc (10);
          sprintf(aEscribir, "%d %d\n", i, SIGUSR1);
          fwrite(aEscribir, 1, sizeof(aEscribir), pfSennales);
          free(aEscribir);
          fclose(pfSennales);
          kill(getppid(), SIGUSR1);
          fprintf(stdout, "Retirando 900 euros de la caja %d\n", i);
        }

        Up_Semaforo(semid, i, SEM_UNDO);

        aleatTime = 1 + rand() % (6);
        sleep(aleatTime);
      }

      fclose(pf[i]);
      Down_Semaforo(semid, n, SEM_UNDO);
      pfSennales = fopen(dirSennalHijo, "w");

      aEscribir = (char *) malloc (10);
      sprintf(aEscribir, "%d %d\n", i, SIGUSR2);

      fwrite(aEscribir, 1, sizeof(aEscribir), pfSennales);

      free(aEscribir);
      fclose(pfSennales);
      kill(getppid(), SIGUSR2);
      fprintf(stdout, "La caja %d ha terminado, retirando dinero restante\n", i);
      exit(EXIT_SUCCESS);

    }else{
      Borrar_Semaforo(semid);
      free(semArray);
      fprintf(stderr, "Ha habido un error en la creacion del hijo numero %d\n", i);
      exit(0);
    }
  }

  Borrar_Semaforo(semid);
  free(semArray);
  exit(EXIT_SUCCESS);

}
