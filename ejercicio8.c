/****************************************
*
* @file ejercicio8.c
*
* @author Ignacio Rabuñal García
* @author Victoria Pelayo Alvaredo
*
* @date 08/03/2018
*
* @brief Crea tantos procesos como programas pasemos
* como argumentos de entrada y ejecuta en casa procesos
* uno de los programas.
*****************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>


#define MAX_PATH 256

int main(int argc, char *argv[]) {

  int i;

  //Definimos dos cadenas de caracteres donde se guardará la ruta al directorio donde se encuentra el programa
  char path1[MAX_PATH];
  char path2[MAX_PATH];

  if(argc < 3){
    printf("Error en los argumentos de entrada\n");
    exit(EXIT_FAILURE);
  }

  if(fork()){
  	wait(NULL);
  }else{
    //Creamos tantos procesos como programas hayamos pasado por linea de comandos.
    for(i = 1; i < argc - 1; i++){
      if(fork()){
      	wait(NULL);
      }else{
        //En funcion del último argumento utilizamos una funcion de la familia "exec" diferente.
      	if(strcmp(argv[argc-1],"-l") == 0){
          sprintf(path1, "/bin/%s", argv[i]);
          sprintf(path2, "/usr/bin/%s",argv[i]);
          execl(path1, argv[i], (char*) NULL);
          execl(path2, argv[i], (char*) NULL);
          perror("Error en execl");
          exit(EXIT_FAILURE);

        }else if(strcmp(argv[argc-1],"-lp") == 0){
          execlp(argv[i], argv[i], (char*) NULL);
          perror("Error en execlp");
          exit(EXIT_FAILURE);

        }else if(strcmp(argv[argc-1],"-v") == 0){
          char *vector[] = {argv[i], NULL};
          sprintf(path1, "/bin/%s", argv[i]);
          sprintf(path2, "/usr/bin/%s",argv[i]);
          execv(path1, vector);
          execv(path2, vector);
          perror("Error en execv");
          exit(EXIT_FAILURE);

        }else if(strcmp(argv[argc-1],"-vp") == 0) {
          char *vector[] = {argv[i], NULL};
          execvp(argv[i],vector);
          perror("Error en execvp");
          exit(EXIT_FAILURE);

      	}
    	}
    }
  }

  return 0;
}
