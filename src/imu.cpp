#include <Wire.h>
#include <Arduino.h>
#include "imu.h"

constexpr uint8_t IMU_ADDRESS = 0x6A;

bool isIMUConnected() { 
    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(0x0F);

    if (Wire.endTransmission(false) != 0) {
        return false;
    }

    if (Wire.requestFrom(IMU_ADDRESS, (uint8_t)1) != 1) {
        return false;
    }

    return Wire.read() == 0x6C;
}


// Averaged (over 1000) values 
static float gyroBiasX = 0.009814f;
static float gyroBiasY = 0.003751f;
static float gyroBiasZ = 0.000043f;


static bool writeIMURegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(reg);
    Wire.write(value);

    return Wire.endTransmission() == 0;
}

void initializeIMU() {

    // Enable block-data update and auto-increment
    writeIMURegister(0x12, 0x44);

    // Accelerometer: 104 Hz, +/-2g
    writeIMURegister(0x10, 0x40);

    // Gyroscope: 104 Hz, +/-250 degrees/s
    writeIMURegister(0x11, 0x40);
}

IMUData readIMUData() {

    uint8_t buffer[12];

    Wire.beginTransmission(IMU_ADDRESS);
    Wire.write(0x22);

    if (Wire.endTransmission(false) != 0) {
        return {NAN, NAN, NAN, NAN, NAN, NAN};
    }

    if (Wire.requestFrom(IMU_ADDRESS, (uint8_t)12) != 12) {
        return {NAN, NAN, NAN, NAN, NAN, NAN};
    }

    for (int i = 0; i < 12; i++) {
        buffer[i] = Wire.read();
    }

    int16_t rawGX = (int16_t)(buffer[0]  | (buffer[1]  << 8));
    int16_t rawGY = (int16_t)(buffer[2]  | (buffer[3]  << 8));
    int16_t rawGZ = (int16_t)(buffer[4]  | (buffer[5]  << 8));

    int16_t rawAX = (int16_t)(buffer[6]  | (buffer[7]  << 8));
    int16_t rawAY = (int16_t)(buffer[8]  | (buffer[9]  << 8));
    int16_t rawAZ = (int16_t)(buffer[10] | (buffer[11] << 8));

    constexpr float ACCEL_SCALE = 0.000061f * 9.80665f;
    constexpr float GYRO_SCALE  = 0.00875f * PI / 180.0f;

    IMUData data;

    data.ax = rawAX * ACCEL_SCALE;
    data.ay = rawAY * ACCEL_SCALE;
    data.az = rawAZ * ACCEL_SCALE;

    data.gx = rawGX * GYRO_SCALE - gyroBiasX;
    data.gy = rawGY * GYRO_SCALE - gyroBiasY;
    data.gz = rawGZ * GYRO_SCALE - gyroBiasZ;
    
return data;
}




