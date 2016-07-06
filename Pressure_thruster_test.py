import serial
import time
import random
ser = serial.Serial('/dev/ttyACM0',57600)

while True:
    # time.sleep(.3)
    # ser.write(random.choice('abcd'))
    ser.write('a')
    print 'test1'
    print ser.readline()
    print 'test2'

ser.close()
