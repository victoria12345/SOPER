###########################################################
CC=gcc
CFLAGS= -g -Wall -pedantic
EJS = ejercicio2 ejercicio2_solved semaforos.o ejercicio3 ejercicio4 ejercicio5
###########################################################
all: $(EJS)

ejercicio2: ejercicio2.c
	$(CC) $(CFLAGS) -o ejercicio2 ejercicio2.c

ejercicio2_solved: ejercicio2_solved.c
	$(CC) $(CFLAGS) -o ejercicio2_solved ejercicio2_solved.c

ejercicio3: ejercicio3.c semaforos.o
	$(CC) $(CFLAGS) -o ejercicio3 ejercicio3.c semaforos.o

ejercicio4: ejercicio4.c
	$(CC) $(CFLAGS) -o ejercicio4 ejercicio4.c -lpthread

ejercicio5: cadena_montaje.c
	$(CC) $(CFLAGS) -o ejercicio5 cadena_montaje.c -lm


semaforos.o: semaforos.c semaforos.h
	$(CC) $(CFLAGS) -c semaforos.c

## limpiamos los .o y ejecutables
clean:
	rm -rf *.o $(EJS)

#comprimimos
dist:
	tar -cvzf G2201_P3_3 *.c *.h memoria.pdf

#Documentacion
doxygen:
	doxygen -g
	doxygen Doxyfile