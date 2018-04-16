#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/uio.h>

typedef struct{
	int fichero;
	pthread_t hilo;
}hfichero;

int aleatorio(int min, int max);
void *escribir_fichero(void* hfichero);
void *leer_fichero(void* hfichero);

int main(int argc, char const *argv[]){
	hfichero *hescribir = (hfichero*)malloc(sizeof(hfichero));
	hfichero *hleer =(hfichero*)malloc(sizeof(hfichero));

	hescribir->fichero = open("numeros.txt", O_RDWR | O_CREAT, 0x0777);
	hleer->fichero = hescribir->fichero;

	if(hescribir->fichero == -1){
		fprintf(stderr, "Error al abrir el fichero\n");
		free(hescribir);
		free(hleer);
		exit(EXIT_FAILURE);
	}

	pthread_create(&hescribir->hilo, NULL, escribir, (void*)hescribir);
	pthread_join(hescribir->hilo, NULL);

	pthread_create(&hleer->hilo, NULL, leer, (void*)hleer);
	pthread_join(hleer->hilo, NULL);

	close(hleer->fichero);
	free(hescribir);
	free(hleer);

	exit(EXIT_SUCCESS);
	
	return 0;
}

int aleatorio(int min, int max){
	return min + rand()%(max-min);
}

void *escribir_fichero(void* hfichero){
	int i;
	int n;
	char *caux;

	if(hfichero == NULL){
		fprintf(stderr, "Error en los argumentos\n");
		pthread_exit(NULL);
	}

	n = aleatorio(1000, 2000);
	caux = (char*)malloc(5);

	for(i = 0; i < n; i++){
		if(i < n - 1){
			sprintf(caux, "%d," aleatorio(100, 1000));
			if(write(((hfichero*)hfichero)->fichero), caux, strlen(caux) == -1){
				fprintf(stderr, "Error al escribir\n");
				free(caux);
				pthread_exit(NULL);
			}
		}else{
			sprintf(caux, "%d\n" aleatorio(100, 1000));
			if(write(((hfichero*)hfichero)->fichero), caux, strlen(caux) == -1){
				fprintf(stderr, "Error al escribir\n");
				free(caux);
				pthread_exit(NULL);
			}
		}
	}

	free(aux);
	pthread_exit(NULL);
}

void *leer_fichero(void* hfichero){
	int i;
	char *ruta;
	struct stat buf;

	if(hfichero == NULL){
		fprintf(stderr, "Error en los argumentos\n");
		pthread_exit(NULL);
	}

	if(fstat(((hfichero*)hstruct)->fichero, &buf) == -1){
    	fprintf(stderr, "Error al extraer tamaño del fichero\n");
  	}

  	ruta = mmap(NULL, buf.st_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, ((hilo_fichero*)hilo_struct)->f, 0);
  	if(ruta == MAP_FAILED){
    	fprintf(stderr, "Error al realizar el mapeo\n");
    	pthread_exit(NULL);
 	}

 	for (i = 0; ruta[i] != '\n'; ++i){
 		if(ruta[i] == ','){
 			ruta[i] = ' ',
 		}
 	}

 	fprintf(stout, "%s\n", ruta);

 	if(munmap(path, buf.st_size) == -1){
 		fprintf(stderr, "Error al liberar el mapeo\n");
 		pthread_exit(NULL);
 	}

 	pthread_exit(NULL);
}