//This example uses the Arduino Servo library to control the
//speed controller. This provides an update rate of 50 Hz and
//can use any pin on the Arduino board as the “servoPin”.

//Note: If you power the Arduino before powering the ESC,
//then the ESC will miss the initialization step and won’t start.
//Power them up at the same time, power the ESC first,
//or press “reset” on the Arduino after applying power 
//to the ESC.

#include <Servo.h>

byte servoPin = 9;
Servo servo;

void setup() {
  servo.attach(servoPin);

  servo.writeMicroseconds(1500); // send "stop" signal to ESC.
  delay(1000); // delay to allow the ESC to recognize the stopped signal
}

void loop() {
  int signal = 1700; // Set signal value, which should be between 1100 and 1900

  servo.writeMicroseconds(signal); // Send signal to ESC.
}
