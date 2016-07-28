#include <Servo.h>

byte servoPin1 = 9;
Servo servo1;

byte servoPin2 = 10;
Servo servo2;


byte servoPin3 = 11;
Servo servo3;


byte servoPin4 = 12;
Servo servo4;



void setup() {
servo1.attach(servoPin1);
servo2.attach(servoPin2);
servo3.attach(servoPin3);
servo4.attach(servoPin4);


servo1.writeMicroseconds(1500); // send "stop" signal to ESC.
servo2.writeMicroseconds(1500); 
servo3.writeMicroseconds(1500); 
servo4.writeMicroseconds(1500); 
  delay(1000); // delay to allow the ESC to recognize the stopped signal
}

void loop() {
  int signal = 1550; // Set signal value, which should be between 1100 and 1900

servo1.writeMicroseconds(signal); // Send signal to ESC.
servo2.writeMicroseconds(signal);
servo3.writeMicroseconds(signal);
servo4.writeMicroseconds(signal);
}

