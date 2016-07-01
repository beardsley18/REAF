    
# Extract pressure reading as a numeric 
import decimal
import serial
ser = serial.Serial('/dev/ttyACM1',57600)
# Setting up variables
    float distance = 0.5 # Distance from bottom of robot to middle hole of sensor
    float initial = 14.73 # Initial pressure above water
    float six_inches = 14.87 # Pressure at 6 in below water 
    float ms_per_reading = 100
    float count = 0
    float average = 0
    float inches = 0
    float total = 0
    
while True:
    count += count
    ptot = ser.readline()
    ptot = float(decimal.Decimal(r))
    pressure = ptot / 15.0

    # Calculate height of fluid above water
    total = total + pressure
    

    if count == ms_per_reading:
        average = total / count
        inches = ((average - initial) / ((six_inches - initial)/6)) + distance
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

      
