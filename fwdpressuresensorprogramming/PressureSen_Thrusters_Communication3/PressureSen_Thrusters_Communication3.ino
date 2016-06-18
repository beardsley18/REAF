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

float count;
float total;
float average;
float inches;
float distance = 0.5; //Distance from bottom of robot to middle hole of sensor
float initial = 14.73; //Initial pressure above water
float sixInches = 14.87; //Pressure at 6 inches below the water (horizontal)
float msPerReading = 100;

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
    //  float sensorValue = analogRead(A0);
    //  float pressure = sensorValue/15;
    //  Serial.println(pressure);
    //  delay(2);
    //from Pressure_Inches_In_Bucket.io
    count = ++count;
    float sensorValue = analogRead(A0);
    float pressure = sensorValue/15.0;
    Serial.println("Pressure:");
    Serial.println(pressure);
    Serial.println("Count:");
    Serial.println(count);
    delay(1);
    total = total + pressure;
    Serial.println("Total:");
    Serial.println(total);
    if (count == msPerReading) {
       average = total / count;
       inches = ((average - initial) / ((sixInches - initial)/6)) + distance;
       Serial.print("Average:");
       Serial.println(average);
       Serial.print("Inches Below Surface:");
       Serial.println(inches);
       count = 0;
       total = 0;
    }
    
    
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
            digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
            delay(1000);              // wait for a second
            digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
            delay(1000);              // wait for a second
    
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
    
            // Pressure Signal
            Serial.println(pressure+1);
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
    
            // Pressure Signal
            Serial.println(pressure+2);
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
  


 

