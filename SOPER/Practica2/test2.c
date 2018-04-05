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

  /*Creamos los ficheros de las cajas*/
  for(i = 0; i < n; i++){
    sprintf(clientesCaja[i], "clientesCaja%d.txt", i);

    clientes[i] = fopen(clientesCaja[i], "w");
    if(!clientes[i]){
      printf("Error abriendo el fichero clientesCaja%d.txt", i);
      exit(ERROR);
    }

    for(j = 0; j < 50; j++){
      fprintf(clientes[i],"%d\n" ,aleatorio(1,300));
    }

    fclose(clientes[i]);
  }
}
