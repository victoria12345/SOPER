###########################################################
CC=gcc
CFLAGS= -g -Wall -pedantic
EJS = ejercicio2 ejercicio4 ejercicio6a ejercicio6b semaforos.o
###########################################################
all: $(EJS)

ejercicio2: ejercicio2.o
	$(CC) $(CFLAGS) -o ejercicio2 ejercicio2.o

ejercicio2.o: ejercicio2.c
	$(CC) $(CFLAGS) -c ejercicio2.c

ejercicio4: ejercicio4.o
	$(CC) $(CFLAGS) -o ejercicio4 ejercicio4.o

ejercicio4.o: ejercicio4.c
	$(CC) $(CFLAGS) -c ejercicio4.c

ejercicio6a: ejercicio6.o
	$(CC) $(CFLAGS) -o ejercicio6a ejercicio6.o

ejercicio6.o: ejercicio6.c
	$(CC) $(CFLAGS) -c ejercicio6.c

ejercicio6b: ejercicio6b.o
	$(CC) $(CFLAGS) -o ejercicio6b ejercicio6b.o

ejercicio6b.o: ejercicio6b.c
	$(CC) $(CFLAGS) -c ejercicio6b.c

semaforos.o: semaforos.c semaforos.h
	$(CC) $(CFLAGS) -c semaforos.c



## limpiamos los .o y ejecutables
clean:
	rm -rf *.o $(EJS)

#comprimimos
dist:
	tar -cvzf G2201_P3_2 *.c *.h memoria.pdf
