#include <Arduino.h>
#include "bmp.h"
// put function declarations here:



void setup() {
  Serial.begin(115200); // Start serial communication at 115200 baud
  initializeBMP();

  if (isBMPConnected()) {
    Serial.println("BMP is connected");
  } else {
    Serial.println("BMP is not connected");
  }


}

void loop() {
  Serial.println(readPressure());
  Serial.println(readTemperature());
  delay(1000);
  
}
