#include <Servo.h>

byte servoPin1 = 9;
Servo servo1;

byte servoPin2 = 10;
Servo servo2;


byte servoPin3 = 11;
Servo servo3;


byte servoPin4 = 12;
Servo servo4;

char inByte;

void setup() {
  Serial.begin(57600);
  Serial.println("Starting");
  
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
  void pressure(); {
    float sensorValue = analogRead(A0);
    float pressure = sensorValue/15;
    Serial.println(pressure);
    delay(1);
  }

void thrusters(); {
  if(Serial.available() > 0){
    char inByte = Serial.read();
    //Serial.println("Beginning of loop");


  if(inByte == 'a') {
    int signal = 40; // Set signal value, which should be between 1100 and 1900
    servo1.writeMicroseconds(signal); // Send signal to ESC.
    Serial.println("Receiving a");
    }
    
   else if(inByte != 'a') {
    int signal = 1500; // Set signal value, which should be between 1100 and 1900
    servo1.writeMicroseconds(signal); // Send signal to ESC.
    Serial.println("Receiving none");
  }

//void serialFlush(); {
  //while(Serial.available() > 0) {
    //signal = Serial.read();
  //if(inByte != 'a') {
    //int signal = 0; // Set signal value, which should be between 1100 and 1900
    //servo1.writeMicroseconds(signal); // Send signal to ESC.
    //Serial.println("Sending a");
 // }
  }
  
}
}





