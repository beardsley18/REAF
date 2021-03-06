/* Blue Robotics Example Code
-------------------------------
 
Title: BlueESC Control via I2C (Arduino)

Description: This example code demonstrates the I2C communication capability
of the Blue Robotics "BlueESC". Motor speed commands are sent via I2C and 
voltage, current, rpm, and temperature data is returned.

The code is designed for the Arduino Uno board and can be compiled and 
uploaded via the Arduino 1.0+ software.

-------------------------------
The MIT License (MIT)

Copyright (c) 2015 Blue Robotics Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-------------------------------*/

//#define TWI_FREQ 100000l // Can be changed to reduce I2C frequency

#include <Wire.h>
#include "Arduino_I2C_ESC.h"

#define ESC_ADDRESS1 0x29
#define ESC_ADDRESS2 0x29
#define ESC_ADDRESS3 0x29
#define ESC_ADDRESS4 0x29

Arduino_I2C_ESC motor1(ESC_ADDRESS1);
Arduino_I2C_ESC motor2(ESC_ADDRESS2);
Arduino_I2C_ESC motor3(ESC_ADDRESS3);
Arduino_I2C_ESC motor4(ESC_ADDRESS4);

int signal;

void setup() {
  Serial.begin(57600);
  Serial.println("Starting");

  Wire.begin();
  motor1.setPWM(1500);
  motor2.setPWM(1500);
  motor3.setPWM(1500);
  motor4.setPWM(1500);

  // Optional: Add these two lines to slow I2C clock to 12.5kHz from 100 kHz
  // This is best for long wire lengths to minimize errors
  TWBR = 158;  
  TWSR |= bit (TWPS0);
}

void loop() {
    // put your main code here, to run repeatedly:
  float sensorValue = analogRead(A0);
  float pressure = sensorValue/15;
  Serial.println(pressure);
  delay(1);

  if ( Serial.available() ) {
    char inByte = Serial.read();  
      // char inByte = ' ';
  // if(Serial.available()) {
    // char inByte = Serial.read();
    // Serial.println(inByte);

    if(inByte == 'a') {
      signal = 1550;
      motor1.setPWM(signal);
      motor2.setPWM(signal);
      motor3.setPWM(signal);
      motor4.setPWM(signal);
      
      motor1.update();
      Serial.print("ESC1: ");
      if(motor1.isAlive()) Serial.print("OK\t\t"); 
      else Serial.print("NA\t\t");
      Serial.print(signal);Serial.print(" \t\t");  
      Serial.print(motor1.rpm());Serial.print(" RPM\t\t");
      Serial.print(motor1.voltage());Serial.print(" V\t\t");
      Serial.print(motor1.current());Serial.print(" A\t\t");
      Serial.print(motor1.temperature());Serial.print(" `C");
      Serial.println();

      
      delay(250); // Update at roughly 4 hz for the demo
    }


}
  }

