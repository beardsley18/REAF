//This code calculates pressure in PSI



void setup() {
  Serial.begin(9600);
Serial.println("Hi");
}
 float count;
float total;
float average;
float inches;
float distance = 0.5; //Distance from bottom of robot to middle hole of sensor
float initial = 14.73; //Initial pressure above water
float sixInches = 14.87; //Pressure at 6 inches below the water (horizontal)
float msPerReading = 100;

void loop() {
  // put your main code here, to run repeatedly:
 
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
}
