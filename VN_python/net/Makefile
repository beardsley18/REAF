DEFINES = LINUX;USE_BUILTIN_SERIALPORT
BINDIR = bin

SOURCES = \
    core/Linux.cs \
    core/DateTimePrecise.cs \
    core/Math/vecf.cs \
    core/Math/vec3f.cs \
    core/Math/vec3d.cs \
    core/Math/vec4f.cs \
    core/Math/matf.cs \
    core/Math/mat3f.cs \
    core/Math/Conv.cs \
    core/Math/Const.cs \
    core/Communication/ISimplePort.cs \
    core/Communication/SerialPort.cs \
    core/Data/Util.cs \
    core/Data/Integrity/Detection.cs \
    core/Protocol/Uart/Common.cs \
    core/Protocol/Uart/Packet.cs \
    core/Protocol/Uart/PacketFinder.cs \
    core/Protocol/Uart/PacketFoundEventArgs.cs \
    core/Protocol/Uart/Util.cs \
    core/Sensor/VnSensor.cs \
    core/Sensor/SensorErrorException.cs

all: library

library: dirs
	@mcs /target:library -unsafe -out:$(BINDIR)/VectorNav.Core.dll -define:"$(DEFINES)" $(SOURCES)

clean:
	@rm -rf bin

.PHONY: dirs
dirs:
	@mkdir -p $(BINDIR)

