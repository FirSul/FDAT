#include <Arduino.h>

// put function declarations here:

const int ledPin = 48; // Pin number for the LED

void setup() {
  // put your setup code here, to run once:

  pinMode(ledPin, OUTPUT); // Set the LED pin as an output

}

void loop() {
  // put your main code here, to run repeatedly:
  neopixelWrite(ledPin, 255, 0, 0); // Red
  delay(1000);

  neopixelWrite(ledPin, 0, 255, 0); // Green
  delay(1000);

  neopixelWrite(ledPin, 0, 0, 255); // Blue
  delay(1000);

  neopixelWrite(ledPin, 0, 0, 0);   // Off
  delay(1000);
}

// put function definitions here:

