###########################################################
CC=gcc
CFLAGS= -g -Wall -pedantic
EJS = carrera 
###########################################################
all: $(EJS)

carrera: carrera.o semaforos.o caballos.o apuestas.o
	$(CC) $(CFLAGS) -o carrera carrera.o semaforos.o caballos.o apuestas.o -lpthread

carrera.o: carrera.c
	$(CC) $(CFLAGS) -c carrera.c

semaforos.o: semaforos.c semaforos.h
	$(CC) $(CFLAGS) -c semaforos.c

caballos.o: caballos.c caballos.h
	$(CC) $(CFLAGS) -c caballos.c

apuestas.o: apuestas.c apuestas.h
	$(CC) $(CFLAGS) -c apuestas.c -lpthread

clean:
	rm -rf *.o carrera
