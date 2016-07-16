import serial
import time
import random
import threading
ser = serial.Serial('/dev/ttyACM0',57600)

def send(lett):
        ser.write(lett)
        time.sleep(1)

class readThread(threading.Thread):
	def __init__(self, threadID):
	    threading.Thread.__init__(self)
            self.threadID = threadID
	
	def run(self):
            while True:
                outFile = open('output_pressure_thruster_test.txt', 'a')
                outFile.write(ser.readline())
                outFile.close()

class writeThread(threading.Thread):
	def __init__(self, threadID):
	    threading.Thread.__init__(self)
            self.threadID = threadID
	
	def run(self):
            ser.write('b')
            ser.write('a')
            for x in range(0,50000):
                # time.sleep(.3)
                # ser.write(random.choice('abcd'))
                # ser.write('a')
                print 'Sending a'
                print x

            ser.write('b')
            ser.write('a')
            for x in range(0, 50000):
                # ser.write('b')
                print 'Sending b'
                print x

            ser.write('a')
            ser.write('b')
            for x in range(0, 50000):
                print 'Sending both'
                print x

            ser.write('a')
            ser.write('b')
            # ser.write('b')
    
   
thread1 = writeThread(1)
thread2 = readThread(2)
thread1.start()
thread2.start()
