import RPi.GPIO as GPIO
import time

#set GPIO pins
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

#motor is connected to pin 4
motor = 4

GPIO.setup(motor, GPIO.OUT)

#start outputting
GPIO.output(motor, 1)

#wait 10 seconds
time.sleep(10)

#stop outputting
GPIO.output(motor, 0)

#done with GPIO
GPIO.cleanup()