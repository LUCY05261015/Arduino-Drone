#include <SoftwareSerial.h>

SoftwareSerial bleSerial(A0, A1); // RX, TX

void setup()
{
  Serial.begin(9600);
  Serial.println("BLE Shield Test Started!");
  bleSerial.begin(9600);
}

void loop() {
  if(bleSerial.available())
    Serial.write(bleSerial.read());
  if(Serial.available())
    bleSerial.write(Serial.read());
}
