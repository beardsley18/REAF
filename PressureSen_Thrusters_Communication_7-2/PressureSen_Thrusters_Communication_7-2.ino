#include <Firmata.h>
#include <Boards.h>


#include <Wire.h>
#include "Arduino_I2C_ESC.h"

#define ESC_ADDRESS_1 0x2A
#define ESC_ADDRESS_2 0x2B
#define ESC_ADDRESS_3 0x2C
#define ESC_ADDRESS_4 0x2D

Arduino_I2C_ESC motor_1(ESC_ADDRESS_1);
Arduino_I2C_ESC motor_2(ESC_ADDRESS_2);
Arduino_I2C_ESC motor_3(ESC_ADDRESS_3);
Arduino_I2C_ESC motor_4(ESC_ADDRESS_4);

int signal;
int n;                

void setup() {
  Serial.begin(57600);
 

   Wire.begin();
    motor_1.setPWM(1500);
    motor_2.setPWM(1500);
    motor_3.setPWM(1500);
    motor_4.setPWM(1500);

   // initialize the LED pins:
   pinMode(13, OUTPUT);
    

}

void loop() {
  
// Pressure Sensor Reading Input 
  float sensorValue = analogRead(A0);
  float pressure = sensorValue/15;
  Serial.println(pressure);
  delay(2);



// read the sensor:
  if (Serial.available() > 0) {
    int inByte = Serial.read();
    // do something different depending on the character received.
    // The switch statement expects single number values for each case;
    // in this exmaple, though, you're using single quotes to tell
    // the controller to get the ASCII value for the character.  For
    // example 'a' = 97, 'b' = 98, and so forth:

        // Motor 1 (Pressure Dependent)
      
        while(int(inByte == 'a')) {

        signal = 1550;
        motor_1.setPWM(signal);

        motor_1.update();

        Serial.print("ESC_1: ");
        if(motor_1.isAlive()) Serial.print("OK\t\t"); 
        else Serial.print("NA\t\t");
        Serial.print(signal);Serial.print(" \t\t");  
        Serial.print(motor_1.rpm());Serial.print(" RPM\t\t");
        Serial.print(motor_1.voltage());Serial.print(" V\t\t");
        Serial.print(motor_1.current());Serial.print(" A\t\t");
        Serial.print(motor_1.temperature());Serial.print(" `C");

        Serial.println();

        delay(250); // Update at roughly 4 hz

        }
        
        
        // Motor 2 (Pressure Dependent)

        while(int(inByte == 'b')) {
          
        signal = 1575;
        motor_2.setPWM(signal);

        motor_2.update();

        Serial.print("ESC_2: ");
        if(motor_2.isAlive()) Serial.print("OK\t\t"); 
        else Serial.print("NA\t\t");
        Serial.print(signal);Serial.print(" \t\t");  
        Serial.print(motor_2.rpm());Serial.print(" RPM\t\t");
        Serial.print(motor_2.voltage());Serial.print(" V\t\t");
        Serial.print(motor_2.current());Serial.print(" A\t\t");
        Serial.print(motor_2.temperature());Serial.print(" `C");

        Serial.println();

        delay(250); // Update at roughly 4 hz

        }
        
        
        // Motor 3

        while(int(inByte == 'c')) {
          
        signal = 1450;
        motor_3.setPWM(signal);

        motor_3.update();

        Serial.print("ESC_3: ");
        if(motor_3.isAlive()) Serial.print("OK\t\t"); 
        else Serial.print("NA\t\t");
        Serial.print(signal);Serial.print(" \t\t");  
        Serial.print(motor_3.rpm());Serial.print(" RPM\t\t");
        Serial.print(motor_3.voltage());Serial.print(" V\t\t");
        Serial.print(motor_3.current());Serial.print(" A\t\t");
        Serial.print(motor_3.temperature());Serial.print(" `C");

        Serial.println();

        delay(250); // Update at roughly 4 hz
        }
        
        // Motor 4

        while(int(inByte == 'd')) {
          
        signal = 1475;
        motor_4.setPWM(signal);

        motor_4.update();

        Serial.print("ESC_2: ");
        if(motor_4.isAlive()) Serial.print("OK\t\t"); 
        else Serial.print("NA\t\t");
        Serial.print(signal);Serial.print(" \t\t");  
        Serial.print(motor_4.rpm());Serial.print(" RPM\t\t");
        Serial.print(motor_4.voltage());Serial.print(" V\t\t");
        Serial.print(motor_4.current());Serial.print(" A\t\t");
        Serial.print(motor_4.temperature());Serial.print(" `C");
 
        Serial.println();

        delay(250); // Update at roughly 4 hz
        }
        
      
        }
    }
  


 

