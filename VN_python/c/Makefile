CC = gcc
INCLUDES = -I include
CFLAGS = -Wall
OBJDIR = obj
BINDIR = bin

SOURCES = \
    src/vn/error_detection.c \
    src/vn/protocol/uart.c \
    src/vn/protocol/spi.c \
    src/vn/util.c \
    src/vn/math.c \
    src/vn/math/vector.c \
    src/vn/sensors.c \
    src/vn/xplat.c \
    src/vn/error.c \
    src/vn/xplat/serialport.c \
    src/vn/xplat/thread.c \
    src/vn/xplat/criticalsection.c \
    src/vn/xplat/event.c \
    src/vn/xplat/time.c \
    src/vn/sensors/searcher.c \
    src/vn/sensors/ezasyncdata.c \
    src/vn/sensors/compositedata.c

# Set the object file names, with the source directory stripped
# from the path, and the build path prepended in its place
OBJECTS = $(SOURCES:src/%.c=$(OBJDIR)/%.o)

all: dirs library

clean:
	rm -f bin/libvnproglib-c.a
	rm -f $(OBJECTS)
	rm -rf obj/vn

library: libvnproglib-c.a

# Create the directories used in the build
.PHONY: dirs
dirs:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)
	@mkdir -p $(dir $(OBJECTS))

libvnproglib-c.a: $(OBJECTS)
	ar -cvq $(BINDIR)/libvnproglib-c.a $(OBJECTS)

# Source file rules
# After the first compilation they will be joined with the rules from the
# dependency files to provide header dependencies
$(OBJDIR)/%.o: src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
