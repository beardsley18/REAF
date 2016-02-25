import RPi.GPIO as GPIO
import time

#set GPIO pins
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

#led is connected to pin 4
led = 4

GPIO.setup(led, GPIO.OUT)

#start outputting
GPIO.output(led, 1)

#wait 10 seconds
time.sleep(10)

#stop outputting
GPIO.output(led, 0)

#done with GPIO
GPIO.cleanup()