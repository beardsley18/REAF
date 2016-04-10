


void setup() {
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
int sensorValue = analogRead(A0);
int pressure = sensorValue/15;
Serial.println(pressure);
delay(1);
}
