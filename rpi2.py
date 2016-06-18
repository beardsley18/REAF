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
    time.sleep(5)
    count = 0
    linesRead = 0
    ahrsData = open('ahrs_output.txt', 'r')
    line = ahrsData.readline()
    while True:
        # copy ahrs data from ahrs_output.txt
        # for i in range(linesRead-1):
            # line = ahrsData.readline()
        while line != "":
            line = line.strip()
            ahrs.append(line)
            print line
            line = ahrsData.readline()
            linesRead += 1
        #pressure sensor
        try:
            r = ser.readline()
            r = float(decimal.Decimal(r))
            pressure.append(r)
        except:
            pass
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
