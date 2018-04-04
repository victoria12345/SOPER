#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

/**
* @brief main del ejercicio2
*
* @author Victoria Pelayo e Ignacio Rabunnal
*/
int main(int argc, char const *argv[]) {
  int i;
  int pid;

  for (i = 0; i < 4; i++) {
    pid = fork();
    if(pid == 0){
      fprintf(stdout, "Soy el proceso hijo %d\n", getpid());
      sleep(30);
      fprintf(stdout, "Soy el proceso hijo %d y ya me toca terminar", getpid());
      exit(EXIT_SUCCESS);
    }else{
      sleep(5);
      kill(pid, SIGTERM);
    }
  }
  return 0;
}
