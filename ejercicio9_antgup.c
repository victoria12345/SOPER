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
#include "semaforos.h"

#define CUENTA_GLOBAL "cuentaGlobal.txt"
#define SEMKEY 75798

int aleatorio(int inf, int supr){
  if(supr == 0 || inf == 0 || supr < inf){
    return -1;
  }
  return inf + rand() % (supr-inf+1);
}

void captura(int sennal){
  return;
}

int main(int argc, char* argv[]){

  if(argc < 2){
    printf("ERROR: introducir el numero de cajas\n");
    exit(ERROR);
  }

  int n = atoi(argv[1]);
  int i,j;
  int semaforos = n+1;
  int semid, cuenta = 0;
  int global = 0;
  int valor =0 ;
  int * pid;
  char clientesCaja[n][30];
  char cuentaCaja[n][30];
  int status;
  int terminados = 0;
  char fichSennal[10] = "sennal.txt";
  unsigned short *array;
  FILE *clientes[n];
  FILE *cajas[n];
  FILE *sennal;
  FILE *fGlobal;



  array = (unsigned short*)malloc(sizeof(unsigned short));
  for(i = 0; i < n; i++){
    array[i] = 1;
  }

  /*Creamos los semaforos*/
  if(Crear_Semaforo(SEMKEY,semaforos, &semid) == ERROR){
    printf("Error creando el primer semaforo\n");
    exit(ERROR);
  }

  if(Inicializar_Semaforo(semid, array) == ERROR){
    printf("Error al inicializar el semaforo\n");
    Borrar_Semaforo(semid);
    exit(ERROR);
  }

  /*Armamos las sennales*/
  if(signal(SIGUSR1,  captura) == SIG_ERR){
    printf("Error armando la sennal usr1\n");
    Borrar_Semaforo(semid);
    exit(ERROR);
  }

  if(signal(SIGUSR2, captura) == SIG_ERR){
    printf("Error armando la sennal SIGUSR2\n");
    Borrar_Semaforo(semid);
    exit(ERROR);
  }

  /*Creamos los ficheros de las cajas*/
  for(i = 0; i< n; i++){
    sprintf(clientesCaja[i], "clientesCaja%d.txt", n);
    clientes[i] = fopen(clientesCaja[i], "w");
    if(!clientes[i]){
      printf("Error abriendo el fichero clientesCaja%d.txt", i);
      Borrar_Semaforo(semid);
      exit(ERROR);
    }

    for(j = 0; j < 50; j++){
      fprintf(clientes[i],"%d\n" ,aleatorio(1,300));
    }

    sprintf(cuentaCaja[i], "cuentaCaja%d.txt");
    cajas[i] = fopen(cuentaCaja[i],"r");
    if(!cajas[i]){
      Borrar_Semaforo(semid);
      for(j = 0; j<i; j++){
        fclose(clientes[j]);
      }
      exit(ERROR);
    }
  }

  /*creamos los n hijos*/

  for(i = 0; i < n; i++){
    int x;
    int j;
    x = fork();
    /*PADRE: Espera  a una sennal*/
    if( x > 0){

      int pid2;
      int senn;
      pid[i] = x;
      while(terminados < n){
        pause();

        /*Cuando recibe sennal mira en el fichero cual es*/
        sennal = fopen(fichSennal, "r");
        if(!sennal){
          printf("Error al abrir el fichero de sennales\n");
          Borrar_Semaforo(semid);
          for(i  =0 ; i < n; i++){
            fclose(clientes[i]);
            fclose(cajas[i]);
            fclose(sennal);
          }
          exit(ERROR);
        }
        fscanf(sennal, "%d %d", &pid2, &senn);
        fclose(sennal);

        if(senn == SIGUSR1){
          /*Se sacan 900 de la caja*/
          Down_Semaforo(semid, pid2, SEM_UNDO);
          global += 900;
          Up_Semaforo(semid, pid2, SEM_UNDO);
          Up_Semaforo(semid, n, SEM_UNDO);
        }
        /*Sacamos todo el dinero de la caja*/
        else if(senn == SIGUSR2){
          terminados++;
          Down_Semaforo(semid, pid2, SEM_UNDO);
          cajas[pid2] = fopen(cuentaCaja[i], "r");
          if(!cajas[pid2]){
            printf("Error al abrir la caja de un hijo\n");
            Borrar_Semaforo(semid);
            for(i  =0 ; i < n; i++){
              fclose(clientes[i]);
              fclose(cajas[i]);
            }
            exit(ERROR);
          }
          fscanf(cajas[pid2], "%d", x);
          fclose(cajas[pid2]);
          global += valor;

          Up_Semaforo(semid, pid2, SEM_UNDO);
          Up_Semaforo(semid, n, SEM_UNDO);
        }
      }
      printf("El total ganado es %d", global);
        for(j = 0; j < n; j++){
          waitpid(pid[i], &status, WUNTRACED | WCONTINUED);
        }
    }
    /*HIJO: lee el dinero de los clientos y va informando al padre*/
    else if(x == 0){
      clientes[i] = fopen(clientesCaja[i], "r");
      if(!clientesCaja[i]){
        printf("Error abriendo el fichero clientesCaja%d.txt", i);
        Borrar_Semaforo(semid);
        for(i  =0 ; i < n; i++){
          fclose(clientes[i]);
          fclose(cajas[i]);
        }
        exit(ERROR);
      }
      for (j = 0; j < 50; j++){
        /*Escribimos los 50 num aleatoorios*/
        if(Down_Semaforo(semid, i, SEM_UNDO) == -1){
          Borrar_Semaforo(semid);
          for(i  =0 ; i < n; i++){
            fclose(clientes[i]);
            fclose(cajas[i]);
          }
          exit(ERROR);
        }
        fscanf(clientes[i], "%d", &valor);
        cuenta += valor;

        cajas[i] = fopen(cuentaCaja[i], "w");
        if(!cajas[i]){
          printf("Error abriendo el fichero cuentaCaja%d.txt", i);
          Borrar_Semaforo(semid);
          for(i  =0 ; i < n; i++){
            fclose(clientes[i]);
            fclose(cajas[i]);
          }
          exit(ERROR);
        }
        /**ESTA DISTINTO***/
        fprintf(cajas[i],"%d", cuenta);
        fclose(cajas[i]);

        if(cuenta >= 1000){
          cuenta -= 900;
          /*Vamos a acceder al fichero de la sennales*/
          Down_Semaforo(semid, n, SEM_UNDO);
          sennal = fopen(fichSennal, "w");
          if(!sennal){
            Borrar_Semaforo(semid);
            for(i = 0; i < n; i++){
              fclose(clientes[i]);
              fclose(cajas[i]);
            }
          }

          fwrite(sennal, "%d %d\n", i,SIGUSR1);
          kill(getppid(), SIGUSR1);
          printf("Caja %d llama a padre para que cja 900 eurso", i);
        }

        Up_Semaforo(semid, i, SEM_UNDO);

        sleep(aleatorio(1,5));
      }
      fclose(clientes[i]);
      Down_Semaforo(semid, n, SEM_UNDO);
      sennal = fopen(fichSennal, "w");
      if(!sennal){
        Borrar_Semaforo(semid);
        for(i = 0; i < n; i++){
          fclose(clientes[i]);
          fclose(cajas[i]);
        }
      }

      fwrite(sennal,"%d %d\n", i, SIGUSR2);
      fclose(sennal);
      kill(getppid(), SIGUSR2);
      printf("La caja %d ha terminado.",i);
      exit(EXIT_SUCCESS);

    }
    /*x<0*/
    else{
      Borrar_Semaforo(semid);
      exit(EXIT_SUCCESS);
    }
  }

  Borrar_Semaforo(semid);

  return 0;

}
