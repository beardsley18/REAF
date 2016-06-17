CC = gcc
INCLUDES = -I ../../include
CFLAGS = -Wall

all: getting_started

clean:
	rm -f main.o
	rm -f getting_started
	cd ../.. && make clean

getting_started: main.o ../../bin/libvnproglib-c.a
	$(CC) -o getting_started main.o -L ../../bin -lvnproglib-c -lpthread

../../bin/libvnproglib-c.a:
	cd ../.. && make

main.o: main.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

