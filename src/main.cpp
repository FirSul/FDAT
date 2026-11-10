#include <Arduino.h>
#include "bmp.h"
// put function declarations here:



void setup() {
  Serial.begin(115200); 
  initializeBMP();

  if (isBMPConnected()) {
    Serial.println("BMP is connected");
  } else {
    Serial.println("BMP is not connected");
  }


}

void loop() {

    BMPData data = readBMPData();

    Serial.print("Pressure: ");
    Serial.print(data.pressure, 2);
    Serial.println(" Pa");

    Serial.print("Temperature: ");
    Serial.print(data.temperature, 2);
    Serial.println(" C");

    delay(1000);
}