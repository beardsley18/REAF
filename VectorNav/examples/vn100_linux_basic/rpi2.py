"""
RPi 2 test code
currently receives AHRS data and outputs data to a file
"""

import datetime
import decimal
# import serial
import subprocess
import threading

# get time program began running
startTime = datetime.datetime.now()

# lists to store sensor readings
ahrs = []
pressure = []

# set up sensors
# ser = serial.Serial('/dev/ttyACM0',57600)

# keep reading values
def ahrs():
	subprocess.call(["./vn100_linux_basic"])
    
def mainLoop():
    count = 0
    while True:       
        # ahrs (dummy vals)
        count += 1
        # ahrs.append(count)
        '''
        #pressure sensor
        try:
            r = ser.readline()
            r = float(decimal.Decimal(r))
            pressure.append(r)
        except:
            pass
        '''
        pressure.append(count)
        if count >= 10:
            break
        
        
class AhrsThread(threading.Thread):
	def __init__(self, threadID):
		threading.Thread.__init__(self)
        	self.threadID = threadID
	
	def run(self):
		ahrs()

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
    
# get time when while loop terminates    
endTime = datetime.datetime.now()

# clean up sensors

# store data to file
outFile = open('output.txt', 'a')
outFile.write('start: ' + (str(startTime)))
outFile.write('\nAHRS:\n')
for val in ahrs:
    outFile.write(str(val) + '\n')

outFile.write('\nPressure sensor:\n')
for val in pressure:
    outFile.write(str(val) + '\n')

outFile.write('\nend: ' + str(endTime))
outFile.write('\n---------------------------------------------------------------------------------------------------\n')
outFile.close()
