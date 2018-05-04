/****************************************
*
* @file carrera_caballos.c
*
* @author Jose Benjumeda Rubio jose.benjumeda@estudiante.uam.es
* @author Martin de las Heras Moreno martin.delasheras@estudiante.uam.es
*
* @date 28/04/2018
*
* @brief
*
*****************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/shm.h> //Funciones memoria compartida
#include <sys/sem.h> //SEM_UNDO
#include <pthread.h>
//#include <sys/types.h>
//#include <sys/ipc.h>
//#include <errno.h>
//#include <signal.h>
//#include <sys/time.h>
//#include <math.h>
//#include <fcntl.h>
#include "semaforos.h"

#define FILEKEY "/bin/ls"
#define KEY 33
#define SEMKEY 0

#define MAX_W 5
#define WORD_MAX 64

#define ANTES 2
#define DURANTE 1
#define DESPUES 0

int estado_carrera = ANTES;       /*!< indicador de si la carrera ha terminado */

int semid;                        /*!< id del array de semaforos */
int num_semaforos = 3;            /*!< numero de semaforos */
unsigned short* array_semaforos;  /*!< array de semaforos */

/** Estructura de un mensaje */

typedef struct _Mensaje{
  long id;        /*!< id del tipo de mensaje */
  int resultado;  /*!< Resultado de tirar el dado */
}mensaje;

/** Estructura de una apuesta */

typedef struct _Apuesta{
  long id;          /*!< id del tipo de mensaje */
  char nombre[20];  /*!< nombre del apostador */
  int caballo;      /*!< numero del caballo */
  double apuesta;   /*!< cantidad apostada */
}apuesta;

/** Estructura utilizada en la memoria compartida */

typedef struct _Shared_Memory{
  double* apuestas;           /*!< array con las apuestas hechas para cada caballo */
  int total_apuestas;         /*!< cantidad total de apuestas hechas */
  double* cotizacion_caballo; /*!< array con las cotizaciones de cada caballo */
}shared_memory;

/** Estructura utilizada para la funcion del hilo */

typedef struct _Struct_Hilo{
  int id;       /*!< id de la ventanilla */
  int id_zone;  /*!< id de la zona de memoria compartida */
  int msqid;    /*!< id de la cola de mensajes */
  int caballos; /*!< numero de caballos */
}estructura_hilo;

/**
 * @brief handler que saca de pausa
 *
 * @param senyal tipo int
 */

void handler_return(int senyal){
  return;
}

/**
 * @brief handler que termina una funcion
 *
 * @param senyal tipo int
 */

void handler_exit(int senyal){
  exit(EXIT_SUCCESS);
}

/**
 * @brief un dado
 *
 * @param max maximo del dado tipo int
 *
 * @return un numero entre 1 y max
 */

int dado(int max){
  return rand()%max + 1;
}

/**
 * @brief Funcion del caballo, recibe la posicion, tira en funcion de ella y envia el resultado en un mensaje.
 *
 * @param tipo leido de la tuberia enviada por el padre
 * @param i posicion en el array tanto de caballos como de pipes
 * @param msqid id de la cola de mensajes para poder enviar uno
 *
 * @return -1 en caso de error y 0 en caso contrario
 */

int caballo(int tipo, int i, int msqid){

  int tirada;
  mensaje msg;

  if(tipo == -1){
    fprintf(stderr, "Error leyendo de la tuberia\n");
    return -1;
  }

  if(tipo == 0)
    tirada = dado(7);

  else if(tipo == 1)
    tirada = dado(6);

  else if (tipo == 2)
    tirada = dado(6) + dado(6);

  else
    return -1;

  msg.id = i + 2;
  msg.resultado = tirada;
  msgsnd(msqid, (struct msgbuf*)&msg, sizeof(mensaje) - sizeof(long), IPC_NOWAIT);

  return tirada;
}

/**
 * @brief Calcula la posicion del caballo respecto a los demas
 *
 * @param ranking array de int con los contadores de los caballos
 * @param pos int con la posicion que ocupa el caballo del que queremos sacar la posicion
 *
 * @return 0 si queda primero, 2 si queda ultimo y 1 en caso aparte.
 */

int calcular_posicion(int* ranking, int pos, int max){
  int i;
  int primero = 0;
  int ultimo = ranking[pos];

  for(i = 0; i < max; i++){
    if(ranking[i] > primero) primero = ranking[i];
    if(ranking[i] < ultimo) ultimo = ranking[i];
  }

  if(ranking[pos] == primero) return 0;

  if(ranking[pos] == ultimo) return 2;

  return 1;
}

/**
 * @brief Envia una apuesta aleatoria
 *
 * @param id del apostador que hace la apuesta
 * @param numero de caballos
 * @param dinero inicial que tiene el apostador
 * @param id de la cola de mensajes
 */

void apostador(int id_apostador, int caballos, int dinero_inicial, int msqid){
  apuesta apuesta;

  apuesta.id = 1;
  sprintf(apuesta.nombre, "Apostador-%d", id_apostador);
  apuesta.caballo = rand()%caballos;
  apuesta.apuesta = rand()%dinero_inicial + 1;

  msgsnd(msqid, (struct msgbuf*) &apuesta, sizeof(apuesta) - sizeof(long), IPC_NOWAIT);
}

/**
 * @brief Funcion ejecutada por los hilos que recibe un mensaje de una apuesta, escribe esa informacion en un fichero y actualiza en la memoria compartida la informacion
 *
 * @param estructura de tipo void* ya que es para un hilo que contiene los atributo necesarios
 */

void* ventanilla(void* struct_hilo){
  apuesta apuesta;
  shared_memory* buff = NULL;
  char* aux = NULL;
  FILE* f = NULL;
  int id;

  if(struct_hilo == NULL)
    pthread_exit(NULL);

  id = ((estructura_hilo*)struct_hilo)->id;
  Up_Semaforo(semid, 2, SEM_UNDO);

  buff = (shared_memory*)malloc(sizeof(shared_memory));
  if((buff = shmat(((estructura_hilo*)struct_hilo)->id_zone, (struct shared_memory*)0, 0)) == NULL){
    fprintf(stderr, "Error reservando memoria\n");
    pthread_exit(NULL);
  }

  while (estado_carrera == ANTES) {
    if (msgrcv(((estructura_hilo*)struct_hilo)->msqid, (struct msgbuf*) &apuesta, sizeof(apuesta) - sizeof(long), 1, IPC_NOWAIT) > 0){
      if(apuesta.caballo > ((estructura_hilo*)struct_hilo)->caballos || apuesta.caballo < 0){
        fprintf(stderr, "Numero de caballo invalido\n");
        pthread_exit(NULL);
      }

      if((aux = (char*)malloc(WORD_MAX)) == NULL){
        fprintf(stderr, "Error reservando memoria en ventanilla\n");
        pthread_exit(NULL);
      }

      sprintf(aux, "%s, %d, %d, %lf, %f\n", apuesta.nombre, id, apuesta.caballo, buff->cotizacion_caballo[apuesta.caballo], apuesta.apuesta);

      Down_Semaforo(semid, 1, SEM_UNDO);

      if((f = fopen("registro_apuestas.txt", "a")) == NULL){
        free(aux);
        pthread_exit(NULL);
      }

      fwrite(aux, strlen(aux), 1, f);
      free(aux);
      fclose(f);

      Up_Semaforo(semid, 1, SEM_UNDO);

      Down_Semaforo(semid, 0, SEM_UNDO);

      buff->total_apuestas += apuesta.apuesta;
      buff->apuestas[apuesta.caballo] += apuesta.apuesta;
      buff->cotizacion_caballo[apuesta.caballo] = buff->total_apuestas / buff->apuestas[apuesta.caballo];

      Up_Semaforo(semid, 0, SEM_UNDO);
    }
  }

  pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {

  int i,j;                      /*!< contadores para los bucles for*/

  int caballos;                 /*!< numero de caballos */
  int carrera;                  /*!< longitud de la carrera */
//int ppid = getpid();          /*!< id del proceso padre para no utilizar getppid */
  int* pids;                    /*!< array de los ids de los hijos */
  int* ranking;                 /*!< array de ints con los contadores de los caballos */

  key_t clave;                  /*!< clave para la cola de mensajes */
  int msqid;                    /*!< id de la cola de mensajes */
  mensaje msg;                  /*!< mensaje */

  char* mensaje_pipe;           /*!< cadena de caracteres para los pipes */

  int apostadores;              /*!< numero de apostadores */
  int ventanillas;              /*!< numero de ventanillas */
  int dinero_inicial;           /*!< dinero inicial */

  int id_apostador;             /*!< id del proceso apostador */
  int id_gestor;                /*!< id del proceso gestor */

  pthread_t* hilo;              /*!< array de hilos */
  estructura_hilo* struct_hilo; /*!< estructura para la funcion del hilo */

  key_t key;                    /*!< key de la memoria compartida */
  int id_zone;                  /*!< id de la zona de memoria compartida */
  shared_memory* buff;          /*!< estructura de datos para la memoria compartida */

  if(argc != 6){
    fprintf(stderr, "Error en los argumentos de entrada\n");
    fprintf(stderr, "%s <numero de caballos> <longitud de la carrera> <numero de apostadores> <numero de ventanillas> <dinero por apostante>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  caballos = atoi(argv[1]);
  carrera = atoi(argv[2]);
  apostadores = atoi(argv[3]);
  ventanillas = atoi(argv[4]);
  dinero_inicial = atoi(argv[5]);

  if(caballos > 10 || caballos < 0 || carrera < 6 || apostadores > 100){
    fprintf(stderr, "Argumentos de entrada invalidos\n");
    exit(EXIT_FAILURE);
  }

  if((pids = (int*)malloc(sizeof(int)*caballos)) == NULL){
    fprintf(stderr, "Error reservando memoria para pids\n");
    exit(EXIT_FAILURE);
  }

  if((ranking = (int*)malloc(sizeof(int)*caballos)) == NULL){
    fprintf(stderr, "Error reservando memoria para el ranking\n");
    free(pids);
    exit(EXIT_FAILURE);
  }

  for(i = 0; i < caballos; i++)
    ranking[i] = 0;

  /** Pipes */

  if((mensaje_pipe = (char*)malloc(MAX_W)) == NULL){
    fprintf(stderr, "Error reservando memoria para la cadena de los pipes\n");
    free(pids);
    free(ranking);
    exit(EXIT_FAILURE);
  }

  int pipes[caballos][2];

  for(i = 0; i < caballos; i++)
    if(pipe(pipes[i]) == -1){
      fprintf(stderr, "Error inicializando tuberias\n");
      free(pids);
      free(ranking);
      free(mensaje_pipe);
      exit(EXIT_FAILURE);
    }


  /** Cola de mensajes */

  clave = ftok(FILEKEY, KEY);
  if(clave == (key_t) -1){
    perror("Error al obtener clave para cola mensajes\n");
    free(pids);
    free(ranking);
    free(mensaje_pipe);
    exit(EXIT_FAILURE);
  }

  msqid = msgget (clave, 0666 | IPC_CREAT);
  if (msqid == -1){
    perror("Error al obtener identificador para cola mensajes");
    free(pids);
    free(ranking);
    free(mensaje_pipe);
    exit(EXIT_FAILURE);
  }

  /** Semaforos */

  if(Crear_Semaforo(SEMKEY, num_semaforos, &semid) == ERROR){
    fprintf(stderr, "Error creando los semaforos.\n");
    free(pids);
    free(ranking);
    free(mensaje_pipe);
    exit(EXIT_FAILURE);
  }

  array_semaforos = (unsigned short*)malloc(sizeof(short)*num_semaforos);
  if(array_semaforos == NULL){
    fprintf(stderr, "Error reservando memoria.\n");
    Borrar_Semaforo(semid);
    free(pids);
    free(ranking);
    free(mensaje_pipe);
    exit(EXIT_FAILURE);
  }

  for(i = 0; i < num_semaforos - 1; i++)
    array_semaforos[i] = 1;
  array_semaforos[num_semaforos - 1] = 0;

  if(Inicializar_Semaforo(semid, array_semaforos) == ERROR){
    fprintf(stderr, "Error inicializando los semaforos\n");
    Borrar_Semaforo(semid);
    free(array_semaforos);
    free(pids);
    free(ranking);
    free(mensaje_pipe);
    exit(EXIT_FAILURE);
  }

  /** Memoria compartida */

  key = ftok(FILEKEY, KEY);
  if(key == -1){
    fprintf(stderr, "Error con key\n");
    Borrar_Semaforo(semid);
    free(array_semaforos);
    free(pids);
    free(ranking);
    free(mensaje_pipe);
    exit(EXIT_FAILURE);
  }

  id_zone = shmget (key, sizeof(struct _Shared_Memory), IPC_CREAT | IPC_EXCL|SHM_R | SHM_W);
  if (id_zone == -1) {
    fprintf (stderr, "Error con id_zone \n");
    Borrar_Semaforo(semid);
    free(array_semaforos);
    free(pids);
    free(ranking);
    free(mensaje_pipe);
    exit(EXIT_FAILURE);
  }

  printf ("ID zone shared memory: %i\n", id_zone);

  buff = shmat (id_zone, (struct _Shared_Memory*)0, 0);
  if(buff == NULL){
    fprintf(stderr, "Error reservando memoria\n");
    Borrar_Semaforo(semid);
    free(array_semaforos);
    free(pids);
    free(ranking);
    free(mensaje_pipe);
    exit(EXIT_FAILURE);
  }

  buff->apuestas = (double*)malloc(sizeof(double)*caballos);
  buff->cotizacion_caballo = (double*)malloc(sizeof(double)*caballos);

  /** Hilos */

  hilo = (pthread_t*)malloc(sizeof(pthread_t)*ventanillas);
  if(hilo == NULL){
    fprintf(stderr, "Error reservando memoria para los hilos\n");
    shmdt((struct _Shared_Memory*)buff);
    shmctl(id_zone, IPC_RMID, (struct shmid_ds*)NULL);
    Borrar_Semaforo(semid);
    free(array_semaforos);
    free(pids);
    free(ranking);
    free(mensaje_pipe);
    msgctl (msqid, IPC_RMID, (struct msqid_ds *)NULL);
    exit(EXIT_FAILURE);
  }

  /** Gestor de apuestas */

  struct_hilo = (estructura_hilo*)malloc(sizeof(estructura_hilo));

  if((id_gestor = fork()) == 0){

    if(signal(SIGUSR1, handler_return) == SIG_ERR){
      fprintf(stderr, "Error en la captura\n");
      exit(EXIT_FAILURE);
    }

    Down_Semaforo(semid, 0, SEM_UNDO);

    for(i = 0; i < caballos; i++){
      buff->apuestas[i] = 1.0;
      buff->total_apuestas += buff->apuestas[i];
    }

    for(i = 0; i < caballos; i++)
      buff->cotizacion_caballo[i] = buff->total_apuestas / buff->apuestas[i];

    Up_Semaforo(semid, 0, SEM_UNDO);

    struct_hilo->caballos = caballos;
    struct_hilo->id_zone = id_zone;
    struct_hilo->msqid = msqid;

    for(i = 0; i < ventanillas; i++){
      struct_hilo->id = i + 1;
      pthread_create(&hilo[i], NULL, ventanilla, (void*)struct_hilo);
      Down_Semaforo(semid, 2, SEM_UNDO);
    }

    pause();

    estado_carrera = DURANTE;

    for(i = 0; i < ventanillas; i++)
      pthread_join(hilo[i], NULL);

    free(struct_hilo);

    exit(EXIT_SUCCESS);
  }

  /** Apostador */

  if((id_apostador = fork()) == 0){
    srand(getpid());

    if(signal(SIGUSR1, handler_exit) == SIG_ERR){
      fprintf(stderr, "Error en la captura\n");
      exit(EXIT_FAILURE);
    }

    for(i = 0; i < apostadores; i++){
      sleep(1);
      apostador(i + 1, caballos, dinero_inicial, msqid);
    }

    exit(EXIT_SUCCESS);
  }
  else if (id_apostador > 0){
    sleep(15);
    estado_carrera = DURANTE;
    kill(id_apostador, SIGUSR1);
    kill(id_gestor, SIGUSR1);

    waitpid(id_apostador, NULL, WUNTRACED | WCONTINUED);
    waitpid(id_gestor, NULL, WUNTRACED | WCONTINUED);
  }
  else{
    fprintf(stderr, "Error en el fork\n");
    free(hilo);
    shmdt((struct _Shared_Memory*)buff);
    shmctl(id_zone, IPC_RMID, (struct shmid_ds*)NULL);
    Borrar_Semaforo(semid);
    free(array_semaforos);
    free(pids);
    free(ranking);
    free(mensaje_pipe);
    msgctl (msqid, IPC_RMID, (struct msqid_ds *)NULL);
    exit(EXIT_FAILURE);
  }

  /** Caballos */

  for(i = 0; i < caballos; i++){
    pids[i] = fork();

    if(pids[i] < 0){
      fprintf(stderr, "Error creando hijos\n");
      free(hilo);
      shmdt((struct _Shared_Memory*)buff);
      shmctl(id_zone, IPC_RMID, (struct shmid_ds*)NULL);
      Borrar_Semaforo(semid);
      free(array_semaforos);
      free(pids);
      free(ranking);
      free(mensaje_pipe);
      msgctl (msqid, IPC_RMID, (struct msqid_ds *)NULL);
      exit(EXIT_FAILURE);
    }

    if(pids[i] > 0){

      sleep(2);

      /** Envia por pipes la posicion inicial a los hijos  (1 para que haga tirada normal) */
      sprintf(mensaje_pipe, "%d", 1);
      close(pipes[i][0]);
      write(pipes[i][1], mensaje_pipe, strlen(mensaje_pipe));

      kill(pids[i], SIGUSR1);

      /** Si no se han creado todos los caballos continua con el bucle ignorando lo de delante */
      if(i < caballos - 1)
        continue;

      /** Mientras ningun caballo llegue al final va recibiendo los mensajes y actualizando los caballos */
      while (estado_carrera != DESPUES) {
        for(j = 0; j < caballos; j++){
          msgrcv (msqid, (struct msgbuf *) &msg, sizeof(mensaje) - sizeof(long), j + 2, 0);
          ranking[j] += msg.resultado;
          if(ranking[j] >= carrera) estado_carrera = DESPUES;
        }

        for(j = 0; j < caballos; j++){
          sprintf(mensaje_pipe, "%d", calcular_posicion(ranking, j, caballos));
          close(pipes[j][0]);
          write(pipes[j][1], mensaje_pipe, strlen(mensaje_pipe));

          kill(pids[j], SIGUSR1);

          fprintf(stdout, "Jaca %d: %d\n", j + 1, ranking[j]);
        }
        fprintf(stdout, "\n");
      }
      /** Envia SIGUSR2 a los caballos cuando acaba la carrera para que terminen */
      for(j = 0; j < caballos; j++) kill(pids[j], SIGUSR2);
    }
    else{
      /** Inicializa la semilla aleatoria y los handlers */
      srand(getpid());

      if(signal(SIGUSR1, handler_return) == SIG_ERR){
        fprintf(stderr, "Error en el handler 1\n");
        exit(EXIT_FAILURE);
      }

      if(signal(SIGUSR2, handler_exit) == SIG_ERR){
        fprintf(stderr, "Error en el handler 2\n");
        exit(EXIT_FAILURE);
      }

      /** Mientras que dure la carrera, se pausan, y despues leen del pipe y llaman a la funcion caballo */
      while(1){
        pause();

        memset(mensaje_pipe, 0, MAX_W);
        close(pipes[i][1]);
        if(read(pipes[i][0], mensaje_pipe, sizeof(mensaje_pipe)) == -1){
          fprintf(stderr, "Error leyendo del pipe\n");
          exit(EXIT_FAILURE);
        }
        if(caballo(atoi(mensaje_pipe), i, msqid) == -1){
          fprintf(stderr, "Error en un caballo\n");
          exit(EXIT_FAILURE);
        }
      }
    }
  }

  for(i = 0; i < caballos; i++) waitpid(pids[i], NULL, WUNTRACED | WCONTINUED);

  free(hilo);
  free(struct_hilo);
  free(buff->apuestas);
  free(buff->cotizacion_caballo);
  shmdt((struct _Shared_Memory*)buff);
  shmctl(id_zone, IPC_RMID, (struct shmid_ds*)NULL);
  Borrar_Semaforo(semid);
  free(array_semaforos);
  free(pids);
  free(ranking);
  free(mensaje_pipe);
  msgctl (msqid, IPC_RMID, (struct msqid_ds *)NULL);

  exit(EXIT_SUCCESS);
}
