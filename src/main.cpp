#include <Arduino.h>
#include "bmp.h"
#include <Wire.h>
#include "imu.h"
// put function declarations here:




void setup() {
  Serial.begin(115200); 

  delay(5000); // wait for connection so setup prints appear in SM
  initializeBMP();
  initializeIMU();

  if (isIMUConnected()) {
    Serial.println("IMU is connected");
  } else {
    Serial.println("IMU is not connected");
  }

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


  IMUData imu = readIMUData();

  Serial.print("Acceleration in X: ");
  Serial.print(imu.ax, 3);
  Serial.print(" | ");

  Serial.print("Acceleration in Y: ");
  Serial.print(imu.ay, 3);
  Serial.print(" | ");

  Serial.print("Acceleration in Z: ");
  Serial.println(imu.az, 3);
  
    Serial.print("Angular velocity in X: ");
  Serial.print(imu.gx, 3);
  Serial.print(" | ");

  Serial.print("Angular velocity in Y: ");
  Serial.print(imu.gy, 3);
  Serial.print(" | ");

  Serial.print("Angular velocity in Z: ");
  Serial.println(imu.gz, 3);
  

    delay(1000);
}