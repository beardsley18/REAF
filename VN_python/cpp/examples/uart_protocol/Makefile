CXX = g++
INCLUDES = -I ../../include
CPPFLAGS = -Wall

SOURCES = main.cpp

all: uart_protocol

clean:
	rm -f main.o
	rm -f uart_protocol
	cd ../.. && make clean

uart_protocol: main.o ../../bin/libvnproglib-cpp.a
	$(CXX) -o uart_protocol main.o -L ../../bin -lvnproglib-cpp -lpthread

../../bin/libvnproglib-cpp.a:
	cd ../.. && make

main.o: main.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

