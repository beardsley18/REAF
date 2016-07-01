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

#global variables
float r = 1.03*10^3 # Density of the fluid (kg/m^3)
float g = 9.8  # Acceleration due to gravity (m/s^2)
float patm = 101325  # Atmospheric pressure in pascals (N/m^2)
float count = 0
float average = 0
float ms_per_reading = 100
float total = 0

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
                line = ahrsData.readline()
                lastRead = ahrsData.tell()
        except IOError as e:
            print e
        #pressure sensor
        try:
            r = ser.readline()
            r = float(decimal.Decimal(r))
            count += count
            pressure.append(r)
            # Calculate height of fluid above water
            h = (ptot-patm)/(r * g)  # Height of the fluid above the object
            total = total + h
            average = total / count

            # Send output to thrusters
            if count == ms_per_reading:
                while average > 4:
                    ser.write('b')
                    if not average > 4:
                        break

                while average < 4:
                    ser.write('a')
                    if not average < 4:
                        break

                while average == 4:
                    ser.write('c')
                    ser.write('d')
                    if not average == 4:
                        break
               count = 0
               total = 0
        except:
            pass
        time.sleep(1)
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
