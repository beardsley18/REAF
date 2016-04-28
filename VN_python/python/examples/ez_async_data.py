# Import some libraries from the VectorNav programming library.
from vnpy.protocol.uart import *
from vnpy.sensors import *
from vnpy.xplat import *

if __name__ == '__main__':

    # This example walks through using the EzAsyncData class to easily access
	# asynchronous data from a VectorNav sensor at a slight performance hit which is
    # acceptable for many applications, especially simple data logging.

    # First determine which COM port your sensor is attached to and update the
    # constant below. Also, if you have changed you sensor from the factory
    # default baud rate of 115200, you will need to update the baud rate
    # constant below as well.
    sensor_port = 'COM1'                            # Windows format for physical and virtual (USB) serial port.
    #sensor_port = '/dev/ttyS1'                     # Linux format for physical serial port.
    #sensor_port = '/dev/ttyUSB0'                   # Linux format for virtual (USB) serial port.
    #sensor_port = '/dev/tty.usbserial-FTXXXXXX'    # Mac OS X format for virtual (USB) serial port.
    #sensor_port = '/dev/ttyS0'                     # CYGWIN format. Usually the Windows COM port number minus 1. This could connect to COM1.
    baud_rate = 115200

    # We create and connect to a sensor by the call below.
    ez = EzAsyncData.connect(sensor_port, baud_rate)

    # Now let's display the latest yaw, pitch, roll data at 5 Hz for 5 seconds.
    for x in range(0, 25):

        Thread.sleep_ms(200)

        # This line reads the latest data that has been processed by the EzAsyncData class.
        cd = ez.current_data

        # Make sure that we have some yaw, pitch, roll data.
        if not cd.has_yaw_pitch_roll:
            print('YPR Unavailable.', flush=True)
        else:
            print('Current YPR: {0}'.format(cd.yaw_pitch_roll), flush=True)

    # Most of the asynchronous data handling is done by EzAsyncData but there are times
    # when we wish to configure the sensor directly while still having EzAsyncData do
    # most of the grunt work. This is easily accomplished and we show the process of
    # changing the ASCII asynchronous data output type here.
    try:
        # TODO: Would like to make the ez.sensor access as a property instead of a function.
        ez.sensor().write_async_data_output_type(AsciiAsync.VNYPR)
    except:
        print('Error setting async data output type.')
        exit()

    print('[New ASCII Async Output]')

    # We can now display yaw, pitch, roll data from the new ASCII asynchronous data type.
    for x in range(0, 25):

        Thread.sleep_ms(200)

        cd = ez.current_data

        # Make sure that we have some yaw, pitch, roll data.
        if not cd.has_yaw_pitch_roll:
            print('YPR Unavailable.', flush=True)
        else:
            print('Current YPR: {0}'.format(cd.yaw_pitch_roll), flush=True)

    # The CompositeData structure contains some helper methods for getting data
    # into various formats. For example, although the sensor is configured to
    # output yaw, pitch, roll, our application might need it as a quaternion
    # value. However, if we query the quaternion field, we see that we don't
    # have any data.

    print('HasQuaternion: {0}'.format(ez.current_data.has_quaternion))

    # Uncommenting the line below will cause an exception to be thrown since
    # quaternion data is not available.

    # print('Current Quaternion: {0}'.format(ez.current_data.quaternion))

    # However, the CompositeData structure provides the any_attitude field
    # which will perform the necessary conversions automatically.

    print('[Quaternion from any_attitude]')

    for x in range(0, 25):

        Thread.sleep_ms(200)

        cd = ez.current_data

        if not cd.has_any_attitude:
            print('Attitude Unavailable.', flush=True)
        else:
            print('Current Quaternion: {0}'.format(cd.any_attitude.quat), flush=True)

    # Throughout this example, we have been using ez.current_data to get the most
    # up-to-date readings from the sensor that have been processed. When called, this
    # method returns immediately with the current values, thus the reason we have to
    # put the Thread.sleep_ss(200) in the for loop. Otherwise, we would blaze through
    # the for loop and just print out the same values. This blazing is demonstrated
    # below.

    print('[For Loop Without Sleep]')

    for x in range(0, 25):

        cd = ez.current_data

        if not cd.has_yaw_pitch_roll:
            print('YPR Unavailable.', flush=True)
        else:
            print('Current YPR: {0}'.format(cd.yaw_pitch_roll), flush=True)

    # Often, we would like to get and process each packet received from the sensor.
    # This is not realistic with ez.current_data since it is non-blocking and we
    # would also have to compare each CompositeData structure for changes in the data.
    # However, EzAsyncData also provides the get_next_data() method which blocks until
    # a new data packet is available. The for loop below shows how to output each
    # data packet received from the sensor using get_next_data().

    print('[get_next_data Method]')

    for x in range(0, 25):

        cd = ez.get_next_data()

        if not cd.has_yaw_pitch_roll:
            print('YPR Unavailable.', flush=True)
        else:
            print('Current YPR: {0}'.format(cd.yaw_pitch_roll), flush=True)

    ez.disconnect()
