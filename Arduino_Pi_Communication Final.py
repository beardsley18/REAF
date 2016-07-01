    
# Extract pressure reading as a numeric 
import decimal
import serial
ser = serial.Serial('/dev/ttyACM1',57600)
# Setting up variables
    float r = 1.03*10^3 # Density of the fluid (kg/m^3)
    float g = 9.8  # Acceleration due to gravity (m/s^2)
    float patm = 101325  # Atmospheric pressure in pascals (N/m^2)
    float count = 0
    float average = 0
    float ms_per_reading = 100
    float total = 0
    
while True: 
    ptot = ser.readline()
    ptot = float(decimal.Decimal(r))
    count += count

    # Calculate height of fluid above water
    h = (ptot-patm)/(r * g)  # Height of the fluid above the object
    total = total + h
    average = total / count

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

    

