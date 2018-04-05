/* PRACTICA 2 EJERCICIO 8 */

/****************************************
*
* @file semaforo.h
*
* @author Javier Martinez Rubio
* @author Santiago Valderrabano Zamorano
*
* @date 23/03/2018
*
*****************************************/


#ifndef semaforo_H
#define semaforo_H

#define OK 1
#define ERROR 0

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

int Inicializar_Semaforo(int semid, unsigned short *array);


/***************************************************************
* Nombre: Borrar_Semaforo.
* Descripcion:
    @brief Borra un semaforo.
* Entrada:
*   @param int semid: Identificador del semaforo.
* Salida:
*   @return int: OK si todo fue correcto, ERROR en caso de error.
***************************************************************/

int Borrar_Semaforo(int semid);


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

int Crear_Semaforo(key_t key, int size, int *semid);



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

int Down_Semaforo(int id, int num_sem, int undo);


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

int DownMultiple_Semaforo(int id,int size,int undo,int *active);



/**************************************************************
* Nombre: Up_Semaforo
* Descripcion:
*   @brief Sube el semaforo indicado
* Entrada:
*   @param int semid: Identificador del semaforo.
*   @param int num_sem: Semaforo dentro del array.
*   @param int undo: Flag de modo persistente pese a finalizacion abupta.
* Salida:
*   @param int: OK si todo fue correcto, ERROR en caso de error.
***************************************************************/

int Up_Semaforo(int id, int num_sem, int undo);


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

int UpMultiple_Semaforo(int id, int size, int undo, int *active);


#endif
