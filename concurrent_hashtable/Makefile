all: client.o server.o

server.o:
	gcc -o server.o server.c table.h table.c shm.c -lpthread -lrt

client.o:
	gcc -o client.o client.c shm_header.h shm.c -lpthread -lrt

clean: 
	rm server.o client.o
