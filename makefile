CC=gcc
CFLAGS=-O2 -Wall -pthread

all: servidor_ext cliente

servidor_ext: servidor_ext.c
	$(CC) $(CFLAGS) servidor_ext.c -o servidor

cliente: cliente.c
	$(CC) $(CFLAGS) cliente.c -o cliente

clean:
	rm -f servidor cliente
