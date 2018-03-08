#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>

#define LEER 0
#define ESCRIBIR 1

/**
* Calcula la potencia de un numero
* @param base base
* @param exponente exponente
* @return base elevadoa  exponente
*/

int potencia(int base, int exponente){
	int i;
	int res = 1;

	for (i = 0; i< exponente; i++){
		res = base * res;
	}


	return res;
}

/**
* Calcula la suma de los valores absolutos de dos numeros
* @param op1 primer numero
* @param op2 segundo numero
* @return suma de sus valores absolutos
*/

int absoluto(int op1, int op2){
	int a, b;

	if(op1 >= 0){
		a = op1;
	}else a = -op1;

	if(op2 >= 0){
		b = op2;
	}else b = -op2;

	return a + b;
}

/**
* Calcula el factorial de un numero
* @param op numero cuyo factorial queremos saber
* @return factorial de un numero
*/
int factorial2(int op){
	if(op <= 1){
		return 1;
	}

	return op * factorial2(op-1);
}

/**
* Calcula el factorial de una division
* @param op1 numerador
* @param op2 denominador
* @return factorial del numero
*/
int factorial(int op1, int op2){
	int a = op1 / op2;

	return factorial2(a);
}

/**
* Devuelve un numero combinatorio
* @param op1 numerador, numero de elementos
* @param op2 denominador, de en cuanto en cuanto se toman
* @return combinatorio
*/
int combinatorio(int op1, int op2){
	return factorial2(op1) / (factorial2(op2) * factorial2(op1 - op2));
}

int main(int argc, char const *argv[]){
	int pipe_status1, pipe_status2;
	int fd[8][2];
	char mensaje1[1000] = " ", final[1000] = " ";
	int status;
	pid_t child_pid;
	int i,j;

	if(argc < 2){
		printf("se deben introducir los dos numeros de la forma a,b. Siendo a y b los operandos\n");
		return -1;
	}

	for(i = 0, j = 4; i<= 3 && j<=7; i++, j++){

			int op1, op2,res;
			char *op1_str = NULL, *op2_str = NULL, res_str[100]= "", pid[100]= "" ;
			char mensaje[1000]= "";

			pipe_status1 = pipe(fd[i]);
			pipe_status2 = pipe(fd[j]);

			if(pipe_status1 == -1 || pipe_status2 == -1){
				printf("Error creando las tuberias para el %d hijo", i+1);
				exit(EXIT_FAILURE);
			}

			child_pid = fork();

			if( child_pid == -1){
			printf("Error en el fork num: %d", i+1);
			exit(EXIT_FAILURE);
			}

			/**Si es el padre*/
			if(child_pid != 0){
				close(fd[i][0]);
				write(fd[i][1], argv[1], strlen(argv[1]));
				close(fd[i][1]);

				wait(&status);

				strcpy(final, "");

				close(fd[j][1]);
				memset(final, 0, sizeof(final));
				read(fd[j][0], final, sizeof(final));
				close(fd[j][0]);

				printf("%s", final);

			}else{
				strcpy(mensaje, "");
				/**Recibo mensaje del padre*/
				close(fd[i][1]);
				read(fd[i][0], mensaje1, sizeof(mensaje1));
				close(fd[i][0]);

				/**Corto el mensaje en los numeros*/
				op1_str = strtok(mensaje1, ",");
				op1 = atoi(op1_str);

				op2_str = strtok(NULL, ",");
				op2 = atoi(op2_str);


				sprintf(pid, "%d", getpid());

				strcat(mensaje, "Datos enviados a través de la tubería por el proceso : ");
				strcat(mensaje, pid);
				strcat(mensaje, ". Operando1:  ");
				strcat(mensaje, op1_str);
				strcat(mensaje, ". Operando2: ");
				strcat(mensaje, op2_str);

				if(i == 0){
					res = potencia(op1, op2);

					sprintf(res_str, "%d", res);

					strcat(mensaje, ". Potencia: ");

				}
				else if(i == 1){
					res = factorial(op1, op2);
			
					sprintf(res_str, "%d", res);

					strcat(mensaje, ". Factorial de operando1 / opeando 2: ");
				}
				else if(i == 2){
					res = combinatorio(op1, op2);
			
					sprintf(res_str, "%d", res);

					strcat(mensaje, ". Permutaciones de operando 1 elementos tomadas de operando 2 en operando 2: ");
				}
				else if(i ==3){
					res = absoluto(op1, op2);

					sprintf(res_str, "%d", res);

					strcat(mensaje, ". Suma de valores absolutos: ");
				}


				strcat(mensaje, res_str);
				strcat(mensaje, "\n");

				close(fd[j][0]);
				write(fd[j][1], mensaje, strlen(mensaje));
				close(fd[j][1]);
				exit(EXIT_SUCCESS);
			}

		}
	return 0;
	
}
