###########################
CFLAGS = -g -Wall -c
EXE = ejercicio4A ejercicio4B ejercicio5A ejercicio5B ejercicio6 ejercicio9 ejercicio12A ejercicio12B
###########################

all: $(EXE)

ejercicio4A : ejercicio4A.o
	gcc -o ejercicio4A ejercicio4A.o

ejercicio4A.o : ejercicio4A.c
	gcc $(CFLAGS) ejercicio4A.c


ejercicio4B : ejercicio4B.o
	gcc -o ejercicio4B ejercicio4B.o

ejercicio4B.o : ejercicio4B.c
	gcc $(CFLAGS) ejercicio4B.c


ejercicio5A : ejercicio5A.o
	gcc -o ejercicio5A ejercicio5A.o

ejercicio5A.o : ejercicio5A.c
	gcc $(CFLAGS) ejercicio5A.c


ejercicio5B : ejercicio5B.o
	gcc -o ejercicio5B ejercicio5B.o

ejercicio5B.o : ejercicio5B.c
	gcc $(CFLAGS) ejercicio5B.c


ejercicio6 : ejercicio6.o
	gcc -o ejercicio6 ejercicio6.o

ejercicio6.o : ejercicio6.c
	gcc $(CFLAGS) ejercicio6.c

ejercicio9 : ejercicio9.o
	gcc -o ejercicio9 ejercicio9.o

ejercicio12A : ejercicio12A.o
	gcc -o ejercicio12A ejercicio12A.o 

ejercicio12A.o: ejercicio12A.c
	gcc $(CFLAGS) ejercicio12A.c 

ejercicio12B : ejercicio12B.o
	gcc -o ejercicio12B ejercicio12B.o -lpthread

ejercicio12B.o: ejercicio12B.c
	gcc $(CFLAGS) ejercicio12B.c -lpthread

clean:
	rm -rf *.o 