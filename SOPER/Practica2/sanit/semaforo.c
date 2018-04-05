/* PRACTICA 2 EJERCICIO 8 */

/****************************************
*
* @file semaforo.c
*
* @author Javier Martinez Rubio
* @author Santiago Valderrabano Zamorano
*
* @date 23/03/2018
*
*****************************************/

/***************************************************************************
*
* @brief Este fichero consiste en uns libreria para gestionar Semaforos.
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/shm.h>
#include "semaforo.h"

#define DAIGUAL 69 /* Esto se debe a que cuando hacemos semctl con la
                      bandera SETALL o IPC_RMID no importa el argumento
                      semun. */

/*union semun {
  int val;
  struct semid_ds *semstat;
  unsigned short *array;
} arg;*/


union semun arg;


/***************************************************************
* Nombre: Inicializar_Semaforo.
* Descripcion:
*   @brief Inicializa los semaforos indicados.
* Entrada:
*   @param int semid: Identificador del semaforo.
*   @param unsigned short *array: Valores iniciales.
* Salida:
*   @return int: OK si todo fue correcto, ERROR en caso de error.
************************************************************/


int Inicializar_Semaforo(int semid, unsigned short *array){


  arg.array = array;

  if(semctl (semid, DAIGUAL, SETALL, arg) == -1){
    return ERROR;
  }

  return OK;

}



/***************************************************************
* Nombre: Borrar_Semaforo.
* Descripcion:
    @brief Borra un semaforo.
* Entrada:
*   @param int semid: Identificador del semaforo.
* Salida:
*   @return int: OK si todo fue correcto, ERROR en caso de error.
***************************************************************/

int Borrar_Semaforo(int semid){

  if(semctl (semid, DAIGUAL, IPC_RMID, 0) == -1){
    return ERROR;
  }

  return OK;
}

/***************************************************************
* Nombre: Crear_Semaforo.
* Descripcion:
*   @brief Crea un semaforo con la clave y el tamaño
*          especificado. Lo inicializa a 0.
* Entrada:
*   @param key_t key: Clave precompartida del semaforo.
*   @param int size: Tamaño del semaforo.
* Salida:
*   @return int *semid: Identificador del semaforo.
*   @return int: ERROR en caso de error,0 si ha creado el semaforo,
*                1 si ya estaba creado.
**************************************************************/

int Crear_Semaforo(key_t key, int size, int *semid){

  *semid = semget(key, size, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
  if((*semid == -1) && (errno == EEXIST)){
    *semid=semget(key, size, SHM_R|SHM_W);
  }
  if(*semid == -1){
    perror("semget");
    return ERROR;
  }

  return OK;

}

/**************************************************************
* Nombre: Down_Semaforo
* Descripcion:
*   @brief Baja el semaforo indicado
* Entrada:
*   @param int semid: Identificador del semaforo.
*   @param int num_sem: Semaforo dentro del array.
*   @param int undo: Flag de modo persistente pese a finalización
*                    abrupta.
* Salida:
*   @return int: OK si todo fue correcto, ERROR en caso de error.
***************************************************************/

int Down_Semaforo(int id, int num_sem, int undo){

  struct sembuf sem_oper; /* Para operaciones up y down sobre semáforos */

  sem_oper.sem_num = num_sem;  /* Actuamos sobre el semáforo id de la lista */
  sem_oper.sem_op = -1;   /* Decrementar en 1 el valor del semáforo */
  sem_oper.sem_flg = undo;

  if(semop (id, &sem_oper, 1) == -1){
    return ERROR;
  }

  return OK;
}

/***************************************************************
* Nombre: DownMultiple_Semaforo
* Descripcion:
*   @brief Baja todos los semaforos del array indicado por active.
* Entrada:
*   @param int semid: Identificador del semaforo.
*   @param int size: Numero de semaforos del array.
*   @param int undo: Flag de modo persistente pese a finalización
*                    abrupta.
*   @param int *active: Semaforos involucrados.
* Salida:
*   @return int: OK si todo fue correcto, ERROR en caso de error.
***************************************************************/

int DownMultiple_Semaforo(int id,int size,int undo,int *active){

  int i;
  struct sembuf sem_oper[size];

  for(i = 0; i < size; i++){
    sem_oper[i].sem_num = active[i];
    sem_oper[i].sem_op = -1;
    sem_oper[i].sem_flg = undo;
  }

  if(semop(id, sem_oper, size)== -1){
    return ERROR;
  }

  return OK;

}

/**************************************************************
* Nombre: Up_Semaforo
* Descripcion:
*   @brief Sube el semaforo indicado
* Entrada:
*   @param int semid: Identificador del semaforo.
*   @param int num_sem: Semaforo dentro del array.
*   @param int undo: Flag de modo persistente pese a finalizacion
*                    abupta.
* Salida:
*   @param int: OK si todo fue correcto, ERROR en caso de error.
***************************************************************/

int Up_Semaforo(int id, int num_sem, int undo){

  struct sembuf sem_oper; /* Para operaciones up y down sobre semáforos */

  sem_oper.sem_num = num_sem;  /* Actuamos sobre el semáforo id de la lista */
  sem_oper.sem_op = 1;   /* Decrementar en 1 el valor del semáforo */
  sem_oper.sem_flg = undo;

  if(semop(id, &sem_oper, 1)== -1){
    return ERROR;
  }

  return OK;

}


/***************************************************************
* Nombre: UpMultiple_Semaforo
* Descripcion:
*   @brief Sube todos los semaforos del array indicado por active.
* Entrada:
*   @param int semid: Identificador del semaforo.
*   @param int size: Numero de semaforos del array.
*   @param int undo: Flag de modo persistente pese a finalización abrupta.
*   @param int *active: Semaforos involucrados.
* Salida:
*   @return int: OK si todo fue correcto, ERROR en caso de error.
***************************************************************/

int UpMultiple_Semaforo(int id, int size, int undo, int *active){

  int i;
  struct sembuf sem_oper[size];

  for(i = 0; i < size; i++){
    sem_oper[i].sem_num = active[i];  /* Actuamos sobre el semáforo id de la lista */
    sem_oper[i].sem_op = 1;   /* Decrementar en 1 el valor del semáforo */
    sem_oper[i].sem_flg = undo;
  }

  if(semop(id, sem_oper, size)== -1){
    return ERROR;
  }

  return OK;

}
