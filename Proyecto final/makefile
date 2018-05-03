###########################################################
CC=gcc
CFLAGS= -g -Wall -pedantic
EJS = carrera semaforos.o
###########################################################
all: $(EJS)

carrera: carrera.o
	$(CC) $(CFLAGS) -o carrera carrera.o

carrera.o: carrera.c
	$(CC) $(CFLAGS) -c carrera.c

semaforos.o: semaforos.c semaforos.h
	$(CC) $(CFLAGS) -c semaforos.c

clean:
	rm -rf *.o carrera
