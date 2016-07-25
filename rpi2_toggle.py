"""
RPi 2 test code
currently receives AHRS and pressure sensor data and outputs data to a file
uses pressure sensor data to move Ula 4' below the surface, then move forward
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
ser = serial.Serial('/dev/ttyACM3',57600)
average = 0
count = 0
total = 0
# false if off, true if on
t1_status = False
t2_status = False
t3_status = False
t4_status = False

t1_stat_changed = False
t2_stat_changed = False
t3_stat_changed = False
t4_stat_changed = False

# send character, wait for response from arduino
def send(lett):
    global t1_stat_changed
    global t2_stat_changed
    global t3_stat_changed
    global t4_stat_changed
    global t1_status
    global t2_status
    global t3_status
    global t4_status
    while True:
        ser.write(lett)
        time.sleep(.5)
        if lett == 'a':
            if t1_stat_changed:
                t1_stat_changed = False
                if t1_status:
                    t1_status = False
                else:
                    t1_status = True
                break
        elif lett == 'b':
            if t2_stat_changed:
                t2_stat_changed = False
                if t2_status:
                    t2_status = False
                else:
                    t2_status = True
                break
        elif lett == 'c':
            if t3_stat_changed:
                t3_stat_changed = False
                if t3_status:
                    t3_status = False
                else:
                    t3_status = True
                break
        elif lett == 'd':
            if t4_stat_changed:
                t4_stat_changed = False
                if t4_status:
                    t4_status = False
                else:
                    t4_status = True
                break


class AhrsThread(threading.Thread):
    def __init__(self, threadID):
	threading.Thread.__init__(self)
        self.threadID = threadID
	        
    def run(self):
        lastRead = 0
        while True:
            try:
                ahrsData = open('ahrs_output.txt', 'r')
                ahrsData.seek(lastRead)
                line = ahrsData.readline()
                while line != "":
                    line = line.strip()
                    ahrs.append(line)
                    line = ahrsData.readline()
                    lastRead = ahrsData.tell()
                    ahrsData.close()
            except IOError as e:
                print 'ahrs: ' + str(e)
            
class ThrustersThread(threading.Thread):
    def __init__(self, threadID):
	threading.Thread.__init__(self)
        self.threadID = threadID
	        
    def run(self):
        # Send output to thrusters
        # time.sleep(2)
        ms_per_reading = 100
        global count
        global average
        global total
        # test1 = 0
        while True:
            # print 'thrusters: ' + str(average) + ' ' + str(test)
            if count == ms_per_reading:
                if average > 4:
                    # ser.write('b')
                    send('b')

                if average < 4:
                    # ser.write('a')
                    send('a')

                if average == 4:
                    # ser.write('c')
                    # ser.write('d')
                    send('c')
                    send('d')

                # test1 += 1
                # time.sleep(1)
                # if test1 > 10:
                    # break
                    
                threadLock.aquire(1)
                count = 0
                total = 0
                threadLock.release()

class MainThread(threading.Thread):
    def __init__(self, threadID):
	threading.Thread.__init__(self)
        self.threadID = threadID
	        
    def run(self):
        test = 0
        global average
        global count
        global total
        global t1_stat_changed
        global t2_stat_changed
        global t3_stat_changed
        global t4_stat_changed
        while True:
            #get pressure sensor data
            try:
                # constants for pressure sensor calculations
                # r = 1.03*10^3 # Density of the fluid (kg/m^3)
                r = 1030
                g = 9.8  # Acceleration due to gravity (m/s^2)
                patm = 101325  # Atmospheric pressure in pascals (N/m^2)
                line = ser.readline()
                print line
                if 'Receiving' in line:
                    # print line[len(line)-3]
                    # print '\'' + line + '\''
                    if line[len(line)-3] == 'a':
                        t1_stat_changed = True
                    elif line[len(line)-3] == 'b':
                        t2_stat_changed = True
                    elif line[len(line)-3] == 'c':
                        t3_stat_changed = True
                    elif line[len(line)-3] == 'd':
                        t4_stat_changed = True
                else:
                    ptot = float(decimal.Decimal(line))
                    if (ptot < 25) and (ptot > 14):
                        threadLock.aquire(1)
                        count += 1
                        pressure.append(ptot)
                        # Calculate height of fluid above water
                        h = (ptot-patm)/(r * g)  # Height of the fluid above the object
                        total = total + h
                        average = total / count
                        threadLock.release()
            except Exception as e:
                print 'pressure: ' + str(e)
                time.sleep(1)
                # test += 1
                # if test > 10:
                    # break


threadLock = threading.Lock()        

thread1 = AhrsThread(1)
thread2 = ThrustersThread(2)
thread3 = MainThread(3)

subprocess.call(["rm", "ahrs_output.txt"])
# run C program for getting AHRS data
subprocess.call(["./VectorNav/examples/vn100_linux_basic/vn100_linux_basic"])
thread1.start()
thread2.start()
thread3.start()
thread1.join()
thread2.join()
thread3.join()

'''
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
'''
