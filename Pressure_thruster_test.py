import serial
import time
import random
ser = serial.Serial('/dev/ttyACM0',57600)

# while True:
    # time.sleep(.3)
    # ser.write(random.choice('abcd'))
    # ser.write('a')
    # print ser.readline()
x = 0
while True:
    x += 1
    print x
    if x < 40:
        ser.write('a')
        print 'Sending a'
    elif x>=40 and x <80:
        ser.write('b')
        print 'Sending b'
    else:
        ser.write('a')
        ser.write('b')
        print 'Sending both'

    print ser.readline()
