import sys
sys.path.append('c:/workspace/proglib/python')

# Import some libraries from the VectorNav programming library.
from vnpy.protocol.uart import *
from vnpy.sensors import *
from vnpy.xplat import *

# This is our basic callback handler for notifications of new asynchronous
# data packets received. The caller parameter is a reference to the event
# object the callback was registered with. The packet parameter is an
# encapsulation of the data packet received from the sensor. At this state,
# the packet has already internally been validated and identified as an
# asynchronous data message. However, some processing is required on the
# user side to make sure it is the right type of asynchronous message type
# so we can parse it correctly. The index and timestamp parameters are
# intended for advanced usage and can be safely ignored for now.
def ascii_async_message_received(caller, packet, index, timestamp):

    # Make sure we have an ASCII packet and not a binary packet.
    if packet.type is not Packet.Type.ascii:
        return

    # Make sure we have a VNYPR data packet.
    if packet.determine_ascii_async_type() is not AsciiAsync.VNYPR:
        return

    # We now need to parse out the yaw, pitch, roll data.
    ypr = packet.parse_VNYPR()

    # Now print out the yaw, pitch, roll measurements.
    print('ASCII Async YPR: {0}'.format(ypr))

def ascii_or_binary_async_message_received(caller, packet, index, timestamp):
    # Make sure we have an ASCII packet and not a binary packet.
    if packet.type is Packet.Type.ascii:

        # Make sure we have a VNYPR data packet.
        if packet.determine_ascii_async_type() is AsciiAsync.VNYPR:

            # We now need to parse out the yaw, pitch, roll data.
            ypr = packet.parse_VNYPR()

            # Now print out the yaw, pitch, roll measurements.
            print('ASCII Async YPR: {0}'.format(ypr))

    if packet.type is Packet.Type.binary:

        if packet.is_compatible(
            CommonGroup.time_startup | CommonGroup.yaw_pitch_roll,
            TimeGroup.none,
            ImuGroup.none,
            GpsGroup.none,
            AttitudeGroup.none,
            InsGroup.none):

            # Ok, we have our expected binary output packet. Since there are many
            # ways to configure the binary data output, the burden is on the user
            # to correctly parse the binary packet. However, we can make use of
            # the parsing convenience methods provided by the Packet structure.
            # When using these convenience methods, you have to extract them in
            # the order they are organized in the binary packet per the User Manual.
            timeStartup = packet.extract_uint64();
            ypr = packet.extract_vec3f();
            print('Binary Async TimeStartup: {0}'.format(timeStartup))
            print('Binary Async Ypr: {0}'.format(ypr))

if __name__ == '__main__':

    # This example illustrates the most common techniques for interfacing with
    # a VectorNav sensor using a Python script.

    # First determine which COM port your sensor is attached to and update the
    # constant below. Also, if you have changed you sensor from the factory
    # default baud rate of 115200, you will need to update the baud rate
    # constant below as well.
    sensor_port = 'COM3'                            # Windows format for physical and virtual (USB) serial port.
    #sensor_port = '/dev/ttyS1'                     # Linux format for physical serial port.
    #sensor_port = '/dev/ttyUSB0'                   # Linux format for virtual (USB) serial port.
    #sensor_port = '/dev/tty.usbserial-FTXXXXXX'    # Mac OS X format for virtual (USB) serial port.
    #sensor_port = '/dev/ttyS0'                     # CYGWIN format. Usually the Windows COM port number minus 1. This could connect to COM1.
    baud_rate = 115200

    # Now create a VnSensor object and use it to connect to the sensor.
    s = VnSensor()
    s.connect(sensor_port, baud_rate)

    # Let's query the sensor's model number.
    mn = s.read_model_number()
    print('Model Number: {0}'.format(mn))

    # Get some orientation data from the sensor.
    ypr = s.read_yaw_pitch_roll()
    print('Current YPR: {0}'.format(ypr))

    # Get some orientation and IMU data.
    reg = s.read_yaw_pitch_roll_magnetic_acceleration_and_angular_rates()
    print('Current YPR: {0}'.format(reg.yaw_pitch_roll))
    print('Current Magnetic: {0}'.format(reg.mag))
    print('Current Acceleration: {0}'.format(reg.accel))
    print('Current Angular Rates: {0}'.format(reg.gyro))

    # Let's do some simple reconfiguration of the sensor. As it comes from the
    # factory, the sensor outputs asynchronous data at 40 Hz. We will change
    # this to 2 Hz for demonstration purposes.
    oldHz = s.read_async_data_output_frequency()
    s.write_async_data_output_frequency(2)
    newHz = s.read_async_data_output_frequency()
    print('Old Async Frequency: {0} Hz'.format(oldHz))
    print('New Async Frequency: {0} Hz'.format(newHz))

    # For the registers that have more complex configuration options, it is
    # convenient to read the current existing register configuration, change
    # only the values of interest, and then write the configuration to the
    # register. This allows preserving the current settings for the register's
    # other fields. Below, we change the heading mode used by the sensor.
    vpe = s.read_vpe_basic_control()
    print('Old Heading Mode: {0}'.format(vpe.heading_mode))
    vpe.heading_mode = HeadingMode.absolute
    s.write_vpe_basic_control(vpe)
    vpe = s.read_vpe_basic_control()
    print('New Heading Mode: {0}'.format(vpe.heading_mode))

    # Up to now, we have shown some examples of how to configure the sensor
    # and query for the latest measurements. However, this querying is a
    # relatively slow method for getting measurements since the CPU has to
    # send out the command to the sensor and also wait for the command
    # response. An alternative way of receiving the sensor's latest
    # measurements, without the waiting for a query response, you can configure
    # the library to alert you when new asynchronous data measurements are
    # received. We will illustrate hooking up to our current VnSensor to
    # receive these notifications of asynchronous messages.

    # First let's configure the sensor to output a known asynchronous data
    # message type.
    s.write_async_data_output_type(AsciiAsync.VNYPR)
    async_type = s.read_async_data_output_type()
    print('ASCII Async Type: {0}'.format(async_type))

    # You will need to define a method which has the appropriate signature
    # for receiving notifications. This is implemented with the method
    # ascii_async_message_received. Now we register the callback method with
    # the VnSensor object.
    s.event_async_packet_received.add(ascii_async_message_received)

    # Now sleep for 5 seconds so that our asynchronous callback method can
    # receive and display yaw, pitch, roll packets.
    print('Starting sleep...')
    Thread.sleep_sec(5)

    # Unregister our callback method.
    s.event_async_packet_received.remove(ascii_async_message_received)

    # As an alternative to receiving notifications of new ASCII asynchronous
    # messages, the binary output configuration of the sensor is another
    # popular choice for receiving data since it is compact, fast to parse,
    # and can be output at faster rates over the same connection baud rate.
    # Here we will configure the binary output register and process packets
    # with a new callback method that can handle both ASCII and binary packets.

    # First we create a structure for creating the configuration information
    # for the binary output register to send yaw, pitch, roll data out at 4 Hz.
    # CommonGroupFlag = CommonGroup.time_startup | CommonGroup.yaw_pitch_roll
    bor = BinaryOutputRegister(
        AsyncMode.port1,
        200,
        # Note use of binary OR to configure flags.
        CommonGroup.time_startup | CommonGroup.yaw_pitch_roll,
        TimeGroup.none,
        ImuGroup.none,
        GpsGroup.none,
        AttitudeGroup.none,
        InsGroup.none)

    s.write_binary_output_1(bor)

    s.event_async_packet_received.add(ascii_or_binary_async_message_received)

    print('Starting sleep...')
    Thread.sleep_sec(5)

    s.event_async_packet_received.remove(ascii_or_binary_async_message_received)

    s.disconnect()
