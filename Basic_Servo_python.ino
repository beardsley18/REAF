#include <Servo.h>

byte servoPin1 = 9;
Servo servo1;

byte servoPin2 = 10;
Servo servo2;

byte servoPin3 = 11;
Servo servo3;

byte servoPin4 = 12;
Servo servo4;

int signal1 = 1500;
int signal2 = 1500;
int signal3 = 1500;
int signal4 = 1500;

void setup() {
    Serial.begin(57600);
    
    servo1.attach(servoPin1);
    servo2.attach(servoPin2);
    servo3.attach(servoPin3);
    servo4.attach(servoPin4);

    signal1 = 1500;
    signal2 = 1500;
    signal3 = 1500;
    signal4 = 1500;

    servo1.writeMicroseconds(1500); // send "stop" signal to ESC.
    servo2.writeMicroseconds(1500); 
    servo3.writeMicroseconds(1500); 
    servo4.writeMicroseconds(1500); 
    delay(1000); // delay to allow the ESC to recognize the stopped signal
}

void loop() {
    // Set signal value, which should be between 1100 and 1900
    servo1.writeMicroseconds(signal1); // Send signal to ESC.
    servo2.writeMicroseconds(signal2);
    servo3.writeMicroseconds(signal3);
    servo4.writeMicroseconds(signal4);
    
    //read in from pi
    char inByte = Serial.read();
    //Serial.println(inByte);
    if(inByte == 'a') {
      Serial.println("Receiving a");
      if(signal1 == 1500){
        signal1 = 1550;
      }
      else{
        signal_1 = 1500;
      }
    }
    if(inByte == 'b') {
      Serial.println("Receiving b");
      if(signal1 == 1500){
        signal1 = 1550;
      }
      else{
        signal_1 = 1500;
      }
    }
    if(inByte == 'c') {
      Serial.println("Receiving c");
      if(signal1 == 1500){
        signal1 = 1550;
      }
      else{
        signal_1 = 1500;
      }
    }
    if(inByte == 'd') {
      Serial.println("Receiving d");
      if(signal1 == 1500){
        signal1 = 1550;
      }
      else{
        signal_1 = 1500;
      }
    }
  }
}