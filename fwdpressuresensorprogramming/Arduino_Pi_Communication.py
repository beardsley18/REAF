# Extract pressure reading as a string 
import serial
ser = serial.Serial('/dev/ttyACM1',9600)
while 1:
    ser.readline()

    
# Extract pressure reading as a numeric 
import decimal
import serial
ser = serial.Serial('/dev/ttyACM1',9600)
while True:
    r = ser.readline()
    r = float(decimal.Decimal(r))
    r/2


