CC = gcc
CFLAGS = -Wall

all: libmsocket.a initmsocket user1 user2
	./initmsocket

libmsocket.a: msocket.o
	ar rcs libmsocket.a msocket.o

initmsocket: initmsocket.o msocket.h 
	$(CC) $(CFLAGS) -o initmsocket initmsocket.o -lpthread -lmsocket -L.

user1: user1.c libmsocket.a
	$(CC) $(CFLAGS) -o user1 user1.c -L. -lmsocket -lpthread

user2: user2.c libmsocket.a
	$(CC) $(CFLAGS) -o user2 user2.c -L. -lmsocket -lpthread

msocket.o: msocket.c msocket.h
	$(CC) $(CFLAGS) -c msocket.c

initmsocket.o: initmsocket.c
	$(CC) $(CFLAGS) -c initmsocket.c

clean:
	rm -f *.o *.a *.gch initmsocket user1 user2