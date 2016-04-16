#include <Servo.h>
const int ledPin = 13;
byte servoPin = 9;
Servo servo;

void setup()
{
  servo.attach(servoPin);

  servo.writeMicroseconds(1500); // send "stop" signal to ESC.
  delay(1000); // delay to allow the ESC to recognize the stopped signal
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop()
{
int signal = 1700; // Set signal value, which should be between 1100 and 1900

  
  
  int sensorValue = analogRead(A0);
int pressure = sensorValue/15;
Serial.println(pressure);
delay(1);

  Serial.println(pressure);
  if (Serial.available())
  {
    signal = Serial.read();
     flash(5);
  }

  servo.writeMicroseconds(signal); // Send signal to ESC.
  delay(1000);
}

void flash(int n)
{
  for (int i = 0; i < n; i++)
  {
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    delay(100);
  }
}
