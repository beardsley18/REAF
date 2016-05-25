"""
RPi 2 test code
currently receives AHRS data and outputs data to a file
"""

import datetime

# get time program began running
startTime = datetime.datetime.now()

# lists to store sensor readings
ahrs = []

# set up sensors

# keep reading values
count = 0
while True:
    count += 1
    # ahrs.append(vectnav.getVals())
    ahrs.append(count)
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
    s_val = str(val)
    outFile.write(s_val)
    outFile.write('\n')

outFile.write('\nend: ' + str(endTime))
outFile.write('\n---------------------------------------------------------------------------------------------------\n')
outFile.close()
