"""
RPi 2 test code
currently receives AHRS and pressure sensor data and outputs data to a file
"""

import datetime
import time
import decimal
import serial
import subprocess
import threading

# get time program began running
startTime = datetime.datetime.now()

# lists to store sensor readings
ahrs = []
pressure = []

# set up sensors
ser = serial.Serial('/dev/ttyACM0',57600)

# keep reading values
def ahrsRun():
	subprocess.call(["./VectorNav/examples/vn100_linux_basic/vn100_linux_basic"])

def mainLoop():
    time.sleep(1)
    count = 0
    lastRead = 0
    while True:
        #ahrs
        try:
            ahrsData = open('ahrs_output.txt', 'r')
            ahrsData.seek(lastRead)
            line = ahrsData.readline()
            while line != "":
                line = line.strip()
                ahrs.append(line)
                print line
                line = ahrsData.readline()
                lastRead = ahrsData.tell()
        except IOError as e:
            print e
        #pressure sensor
        try:
            r = ser.readline()
            r = float(decimal.Decimal(r))
            print r
            pressure.append(r)
        except:
            pass
        time.sleep(1)
        count += 1
        if count > 10:
            break
    ahrsData.close()
        
        
class AhrsThread(threading.Thread):
	def __init__(self, threadID):
		threading.Thread.__init__(self)
        	self.threadID = threadID
	
	def run(self):
		ahrsRun()

class MainThread(threading.Thread):
	def __init__(self, threadID):
		threading.Thread.__init__(self)
        	self.threadID = threadID
	
	def run(self):
		mainLoop()


thread1 = AhrsThread(1)
thread2 = MainThread(2)

subprocess.call(["rm", "ahrs_output.txt"])
thread1.start()
thread2.start()
thread1.join()
thread2.join()
    
# get time when while loop terminates    
endTime = datetime.datetime.now()

# clean up sensors


# store data to file
outFile = open('output.txt', 'a')
outFile.write('start: ' + (str(startTime)))
outFile.write('\nAHRS:\n')
for val in ahrs:
    outFile.write(val + '\n')

outFile.write('\nPressure sensor:\n')
for val in pressure:
    outFile.write(str(val) + '\n')

outFile.write('\nend: ' + str(endTime))
outFile.write('\n---------------------------------------------------------------------------------------------------\n')
outFile.close()
