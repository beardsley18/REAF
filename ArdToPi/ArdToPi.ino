const int ledPin = 13;

void setup()
{
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop()
{
  int sensorValue = analogRead(A0);
int pressure = sensorValue/15;
Serial.println(pressure);
delay(1);

  Serial.println(pressure);
  if (Serial.available())
  {
     flash(Serial.read() - '0');
  }
  delay(1000);
}
// This isn’t necessary. Just makes LED flash to confirm connection
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
