CXX = g++
INCLUDES = -I ../../include
CPPFLAGS = -Wall

SOURCES = main.cpp

all: math

clean:
	rm -f main.o
	rm -f math
	cd ../.. && make clean

math: main.o ../../bin/libvnproglib-cpp.a
	$(CXX) -o math main.o -L ../../bin -lvnproglib-cpp

../../bin/libvnproglib-cpp.a:
	cd ../.. && make

main.o: main.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

