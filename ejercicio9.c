#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define LEER 0
#define ESCRIBIR 1

void main(int argc, char const *argv[]){
	int i,j,k;
	int fd1[2], fd2[2], fd3[2], fd4[2], fd5[2], fd6[2], fd7[2], fd8[2];
	int op1, op2;
	char mensaje[100];
	char mensaje_final[100];


	if(argc< 1){
		printf("Falta un operando por introducir");
		return ;
	}

	if(pipe(fd1) != 0){
		printf("Error al crar las tuberias");
		return;
	}

	if(fork() != 0){
		close(fd1[LEER]);
		write(fd1[ESCRIBIR], argv[0], strlen(argv[0]) + 1);
		close(fd1[ESCRIBIR]);
		wait();

	}else{
		int Pid;
		char *pid;
		char op1[10], op2[10];
		int Op1, Op2, resultado;
		char Resultado[100], envio[100];

		close(fd1[ESCRIBIR]);
		read(fd1[LEER], mensaje, 100);
		close(fd1[LEER]);

		for(i = 0; i < len(argv[0]); i++){
			if(argv[0][i] == ','){

				for(k = 0; k<i;k++){
					op1[k] = argv[0][k];
				}

				for(k = i +1; argv[k]!= '\0'; k++){
					op2[k] = argv[0][k];
				}

				Op1 = atoi(op1);
				Op2 = atoi(op2);

				resultado = pow(Op1, Op2);

				Pid = get_pid();
				pid= itoa(Pid);

				/**Resultado = itoa(resultado);*/

				strpy(envio, "Datos enviados a traves de la tuberia por el proceso PID=");
				strcat(envio, pid);
				strcat(envio, "Operando1: ");
				strcat(envio, op1);
				strcat(envio, "Operando2: ");
				strcat(envio, op2);
				strcat(envio, "Potencia: ");
				/**strcat(envio, Resultado);*/

				i = len(argv[0]);

			}
		}

		if(pipe(fd2) != 0){
			printf("Error al crear las tuberias");
		}

		close(fd2[LEER]);
		write(fd2[ESCRIBIR], Resultado, strlen(Resultado) + 1);
		close(fd2[ESCRIBIR]);
		exit(EXIT_SUCCESS);
	}

	/**Leemos lo que envia el hijo 1*/

	close(fd2[ESCRIBIR]);
	read(fd2[LEER], mensaje_final, 100);
	close(fd2[LEER]);

	printf("%s", mensaje_final);
}