#include <Wire.h>
#include "Arduino_I2C_ESC.h"
//#include <Firmata.h>

// #define ESC_ADDRESS 0x29
#define ESC_ADDRESS1 0x2A
#define ESC_ADDRESS2 0x2B
#define ESC_ADDRESS3 0x2C
#define ESC_ADDRESS4 0x2D

//Arduino_I2C_ESC motor(ESC_ADDRESS);
Arduino_I2C_ESC motor1(ESC_ADDRESS1);
Arduino_I2C_ESC motor2(ESC_ADDRESS2);
Arduino_I2C_ESC motor3(ESC_ADDRESS3);
Arduino_I2C_ESC motor4(ESC_ADDRESS4);

int signal;
int signal_1;
int signal_2;
int signal_3;
int signal_4;

void setup() {
  Serial.begin(57600);
  Serial.println("Starting");
  
  Wire.begin();

  // Optional: Add these two lines to slow I2C clock to 12.5kHz from 100 kHz
  // This is best for long wire lengths to minimize errors
  TWBR = 158;
  TWSR |= bit (TWPS0);

  /*for set function:
   * -32767 (max reverse) to 32767 (max forward)
   * 0 is stopped
   * No deadband*/
  motor1.set(0);
  motor2.set(0);
  motor3.set(0);
  motor4.set(0);
}

void loop() {
  if ( Serial.available() > 0 ) {
    //signal = Serial.parseInt();
    signal = 3000;

    char inByte = Serial.read();
    if(inByte == 'a') {
      Serial.println("Receiving a");
      //motor1.set(signal_1);
      //signal_1 = 3000;
      motor1.set(signal);
      
      motor1.update();

      /*if(motor1.isAlive()) Serial.println("OK");
      else Serial.println("NA");
      Serial.print(String(signal_1) + "\t" + String(motor1.rpm()) + "RPM\t" + String(motor1.voltage()) + "V\t" + String(motor1.current()) + "A\t" + String(motor1.temperature()) + "C");
      Serial.println();*/
      Serial.print("ESC: ");
      if(motor1.isAlive()) Serial.print("OK\t\t"); 
      else Serial.print("NA\t\t");
      Serial.print(signal_1);Serial.print(" \t\t");  
      Serial.print(motor1.rpm());Serial.print(" RPM\t\t");
      Serial.print(motor1.voltage());Serial.print(" V\t\t");
      Serial.print(motor1.current());Serial.print(" A\t\t");
      Serial.print(motor1.temperature());Serial.print(" `C");
      Serial.println();
  
      delay(250); // Update at roughly 4 hz for the demo
    }  
    else if(inByte == 'b') {
      Serial.println("Receiving b");
      //motor2.set(signal_2);
      //signal_2 = 3000;
      motor2.set(signal);
     
      motor2.update();

      /*if(motor2.isAlive()) Serial.println("OK");
      else Serial.println("NA");
      Serial.print(String(signal_2) + "\t" + String(motor2.rpm()) + "RPM\t" + String(motor2.voltage()) + "V\t" + String(motor2.current()) + "A\t" + String(motor2.temperature()) + "C");
      Serial.println();*/
      Serial.print("ESC: ");
      if(motor2.isAlive()) Serial.print("OK\t\t"); 
      else Serial.print("NA\t\t");
      Serial.print(signal_2);Serial.print(" \t\t");  
      Serial.print(motor2.rpm());Serial.print(" RPM\t\t");
      Serial.print(motor2.voltage());Serial.print(" V\t\t");
      Serial.print(motor2.current());Serial.print(" A\t\t");
      Serial.print(motor2.temperature());Serial.print(" `C");
      Serial.println();
  
      delay(250); // Update at roughly 4 hz for the demo
    }   
    else if(inByte == 'c') {
      Serial.println("Receiving c");
      //motor3.set(signal_3);
      //signal_3 = 3000;
      motor3.set(signal);
      
      motor3.update();

      /*if(motor3.isAlive()) Serial.println("OK");
      else Serial.println("NA");
      Serial.print(String(signal_3) + "\t" + String(motor3.rpm()) + "RPM\t" + String(motor3.voltage()) + "V\t" + String(motor3.current()) + "A\t" + String(motor3.temperature()) + "C");
      Serial.println();*/
      Serial.print("ESC: ");
      if(motor3.isAlive()) Serial.print("OK\t\t"); 
      else Serial.print("NA\t\t");
      Serial.print(signal_3);Serial.print(" \t\t");  
      Serial.print(motor3.rpm());Serial.print(" RPM\t\t");
      Serial.print(motor3.voltage());Serial.print(" V\t\t");
      Serial.print(motor3.current());Serial.print(" A\t\t");
      Serial.print(motor3.temperature());Serial.print(" `C");
      Serial.println();
  
      delay(250); // Update at roughly 4 hz for the demo
    }  
    else if(inByte == 'd') {
      Serial.println("Receiving d");
      //motor4.set(signal_4);
      //signal_4 = 3000;
      motor4.set(signal);
      
      motor4.update();

      /*if(motor4.isAlive()) Serial.println("OK");
      else Serial.println("NA");
      Serial.print(String(signal_4) + "\t" + String(motor4.rpm()) + "RPM\t" + String(motor4.voltage()) + "V\t" + String(motor4.current()) + "A\t" + String(motor4.temperature()) + "C");
      Serial.println();*/
      Serial.print("ESC: ");
      if(motor4.isAlive()) Serial.print("OK\t\t"); 
      else Serial.print("NA\t\t");
      Serial.print(signal_4);Serial.print(" \t\t");  
      Serial.print(motor4.rpm());Serial.print(" RPM\t\t");
      Serial.print(motor4.voltage());Serial.print(" V\t\t");
      Serial.print(motor4.current());Serial.print(" A\t\t");
      Serial.print(motor4.temperature());Serial.print(" `C");
      Serial.println();
  
      delay(250); // Update at roughly 4 hz for the demo
    }
    else{
      Serial.println("None " + inByte);
    }
  }
}


