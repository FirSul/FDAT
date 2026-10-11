#include <Wire.h>
#include <Arduino.h>

void initializeBMP() {
    Wire.begin(8, 9);  // SDA = 8, SCL = 9

    Wire.beginTransmission(0x76);  // BMP388 address
    Wire.write(0x1B);              // PWR_CTRL register
    Wire.write(0x33);              // Enable pressure, temperature, normal mode
    Wire.endTransmission();        // Send the configuration
    
}

bool isBMPConnected() {
  Wire.beginTransmission(0x76);
  return Wire.endTransmission() == 0;

}



uint32_t readPressure() {
    Wire.beginTransmission(0x76);
    Wire.write(0x04);
    Wire.endTransmission(false);

    Wire.requestFrom(0x76,3);
    uint8_t p_xlsb = Wire.read();
    uint8_t p_lsb  = Wire.read();
    uint8_t p_msb  = Wire.read();

    uint32_t rawvalue =
        ((uint32_t)p_msb << 16) |
        ((uint32_t)p_lsb << 8) |
        p_xlsb;

 

    return rawvalue;
}


uint32_t readTemperature() {
    Wire.beginTransmission(0x76);
    Wire.write(0x07);
    Wire.endTransmission(false);

    Wire.requestFrom(0x76,3);
    uint8_t t_xlsb = Wire.read();
    uint8_t t_lsb  = Wire.read();
    uint8_t t_msb  = Wire.read();

    uint32_t rawvalue =
        ((uint32_t)t_msb << 16) |
        ((uint32_t)t_lsb << 8) |
        t_xlsb;

    

    return rawvalue;

}