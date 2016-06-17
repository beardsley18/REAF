"""
RPi 2 test code
currently receives AHRS data and outputs data to a file
"""

import datetime
import decimal
import serial

# get time program began running
startTime = datetime.datetime.now()

# lists to store sensor readings
ahrs = []
pressure = []

# set up sensors
ser = serial.Serial('/dev/ttyACM0',57600)

# keep reading values
count = 0
while True:
    # ahrs (dummy vals)
    count += 1
    ahrs.append(count)
    #pressure sensor
    try:
        r = ser.readline()
        r = float(decimal.Decimal(r))
        pressure.append(r)
    except:
        pass
    if count >= 10:
        break
        
    
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
