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

#define DADO_NORMAL 1
#define SIETE_CARAS 2
#define DOBLE_DADO 3

typedef struct _Mensaje{
	long id; /*!< Tipo del mensaje*/
	int tirada; /*!< Resultado de tirar el dado*/
}Mensaje;

int calcular_tirada(int* array, int pos, int longitud);
int caballo(int tipo_dado, int id_mensajes, int pos);
void manejador_final(int sennal);
void manejador_carrera(int sennal);