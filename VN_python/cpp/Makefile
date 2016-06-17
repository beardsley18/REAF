CXX = g++
INCLUDES = -I include
CPPFLAGS = -Wall
OBJDIR = obj
BINDIR = bin

SOURCES = \
    src/vn/math/conversions.cpp \
    src/vn/math/attitude.cpp \
    src/vn/math/position.cpp \
    src/vn/protocol/uart/types.cpp \
    src/vn/protocol/uart/util.cpp \
    src/vn/protocol/uart/packet.cpp \
    src/vn/protocol/uart/packetfinder.cpp \
    src/vn/sensors/sensors.cpp \
    src/vn/sensors/searcher.cpp \
    src/vn/sensors/compositedata.cpp \
    src/vn/sensors/ezasyncdata.cpp \
    src/vn/util/memoryport.cpp \
    src/vn/xplat/criticalsection.cpp \
    src/vn/xplat/event.cpp \
    src/vn/xplat/serialport.cpp \
    src/vn/xplat/thread.cpp \
    src/vn/xplat/time.cpp \
    src/vn/data/error_detection.cpp \
    src/vn/utilities.cpp \
    src/vn/xplat/port.cpp \
    src/vn/event.cpp

# Set the object file names, with the source directory stripped
# from the path, and the build path prepended in its place
OBJECTS = $(SOURCES:src/%.cpp=$(OBJDIR)/%.o)

all: dirs library

clean:
	rm -f bin/libvnproglib-cpp.a
	rm -f $(OBJECTS)
	rm -rf obj/vn

library: libvnproglib-cpp.a

# Create the directories used in the build
.PHONY: dirs
dirs:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)
	@mkdir -p $(dir $(OBJECTS))

libvnproglib-cpp.a: $(OBJECTS)
	ar -cvq $(BINDIR)/libvnproglib-cpp.a $(OBJECTS)

# Source file rules
# After the first compilation they will be joined with the rules from the
# dependency files to provide header dependencies
$(OBJDIR)/%.o: src/%.cpp
	$(CXX) $(CPPFLAGS) $(INCLUDES) -c $< -o $@
