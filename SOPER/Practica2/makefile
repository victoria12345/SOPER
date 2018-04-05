###########################################################
CC=gcc
CFLAGS= -g -Wall -pedantic
EJS = ejercicio2 ejercicio4 ejercicio6a ejercicio6b semaforos.o test ejercicio9
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

ejercicio6a: ejercicio6a.o
	$(CC) $(CFLAGS) -o ejercicio6a ejercicio6a.o

ejercicio6.o: ejercicio6a.c
	$(CC) $(CFLAGS) -c ejercicio6a.c

ejercicio6b: ejercicio6b.o
	$(CC) $(CFLAGS) -o ejercicio6b ejercicio6b.o

ejercicio6b.o: ejercicio6b.c
	$(CC) $(CFLAGS) -c ejercicio6b.c

semaforos.o: semaforos.c semaforos.h
	$(CC) $(CFLAGS) -c semaforos.c

ejercicio9.0: ejercicio9.c semaforos.h
	$(CC) $(CFLAGS) -c ejercicio9.c

ejercicio9: ejercicio9.o semaforos.o
	$(CC) $(CFLAGS) -o ejercicio9 ejercicio9.o semaforos.o

test.o: test.c semaforos.h
	$(CC) -c  test.c
test: test.o semaforos.o
	$(CC) -o test test.o semaforos.o


## limpiamos los .o y ejecutables
clean:
	rm -rf *.o $(EJS)

#comprimimos
dist:
	tar -cvzf G2201_P3_2 *.c *.h memoria.pdf
