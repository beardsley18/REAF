CC = gcc
INCLUDES = -I ../../include
CFLAGS = -Wall

all: spi

clean:
	rm -f main.o
	rm -f spi
	cd ../.. && make clean

spi: main.o proglib
	$(CC) -o spi main.o -L ../../bin -lvnproglib-c -lpthread

proglib:
	cd ../.. && make

main.o: main.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

