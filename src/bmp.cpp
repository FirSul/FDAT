#include <Wire.h>
#include <Arduino.h>
#include "bmp.h"
constexpr uint8_t BMP_ADDRESS = 0x76;

struct CalibrationData {
    double par_t1;
    double par_t2;
    double par_t3;

    double par_p1;
    double par_p2;
    double par_p3;
    double par_p4;
    double par_p5;
    double par_p6;
    double par_p7;
    double par_p8;
    double par_p9;
    double par_p10;
    double par_p11;
};

static CalibrationData calibration = {};
static bool calibrationLoaded = false;

// forward dec
void readCalibration();

//Register write
static bool writeRegister(uint8_t reg, uint8_t value) {

    Wire.beginTransmission(BMP_ADDRESS);

    Wire.write(reg);
    Wire.write(value);

    return Wire.endTransmission() == 0;
}


// bmp init
void initializeBMP() {
    Wire.begin(8, 9); // SDA = 8, SCL = 9

    // Sleep before config
    if (!writeRegister(0x1B, 0x00)) {
        Serial.println("BMP388 sleep configuration failed");
        return;
    }

    // Pressure oversampling x8, temperature x1
    if (!writeRegister(0x1C, 0x03)) {
        Serial.println("BMP388 oversampling configuration failed");
        return;
    }

    // Output data rate at 25 Hz
    if (!writeRegister(0x1D, 0x03)) {
        Serial.println("BMP388 ODR configuration failed");
        return;
    }

    // Enable pressure, temperature and normal mode
    if (!writeRegister(0x1B, 0x33)) {
        Serial.println("BMP388 normal mode configuration failed");
        return;
    }

    readCalibration();
}

bool isBMPConnected() {
    Wire.beginTransmission(BMP_ADDRESS);
    return Wire.endTransmission() == 0;
}

// Reading register func:
static bool readRawMeasurements(uint32_t& pressure, uint32_t& temperature) {

    Wire.beginTransmission(BMP_ADDRESS);
    Wire.write(0x04);

    if (Wire.endTransmission(false) != 0) {
        return false;
    }

    if (Wire.requestFrom(BMP_ADDRESS, 6) != 6) {
        return false;
    }

    uint8_t p_xlsb = Wire.read();
    uint8_t p_lsb  = Wire.read();
    uint8_t p_msb  = Wire.read();

    uint8_t t_xlsb = Wire.read();
    uint8_t t_lsb  = Wire.read();
    uint8_t t_msb  = Wire.read();

    pressure =
        ((uint32_t)p_msb << 16) |
        ((uint32_t)p_lsb << 8) |
        p_xlsb;

    temperature =
        ((uint32_t)t_msb << 16) |
        ((uint32_t)t_lsb << 8) |
        t_xlsb;

    return true;
}


// Calibration coefficients
void readCalibration() {
    calibrationLoaded = false;

    Wire.beginTransmission(BMP_ADDRESS);
    Wire.write(0x31);
    Wire.endTransmission(false);

    if (Wire.requestFrom(BMP_ADDRESS, 21) != 21) {
        return;
    }

    uint8_t calibrationData[21];

    for (int i = 0; i < 21; i++) {
        calibrationData[i] = Wire.read();
    }

    // temp params

    uint16_t par_t1 =
        ((uint16_t)calibrationData[1] << 8) | calibrationData[0];

    uint16_t par_t2 =
        ((uint16_t)calibrationData[3] << 8) | calibrationData[2];

    int8_t par_t3 = (int8_t)calibrationData[4];

    // temperature scaling equations

    calibration.par_t1 = (double)par_t1 * 256.0;
    calibration.par_t2 = (double)par_t2 / 1073741824.0;
    calibration.par_t3 = (double)par_t3 / 281474976710656.0;


    // pressure params

    int16_t par_p1 =
        (int16_t)(((uint16_t)calibrationData[6] << 8) |
        calibrationData[5]);

    int16_t par_p2 =
        (int16_t)(((uint16_t)calibrationData[8] << 8) |
        calibrationData[7]);

    int8_t par_p3 = (int8_t)calibrationData[9];

    int8_t par_p4 = (int8_t)calibrationData[10];

    uint16_t par_p5 =
        ((uint16_t)calibrationData[12] << 8) | calibrationData[11];

    uint16_t par_p6 =
        ((uint16_t)calibrationData[14] << 8) | calibrationData[13];

    int8_t par_p7 = (int8_t)calibrationData[15];

    int8_t par_p8 = (int8_t)calibrationData[16];

    int16_t par_p9 =
        (int16_t)(((uint16_t)calibrationData[18] << 8) |
        calibrationData[17]);

    int8_t par_p10 = (int8_t)calibrationData[19];

    int8_t par_p11 = (int8_t)calibrationData[20];


    // pressure scaling equations

    calibration.par_p1 = ((double)par_p1 - 16384.0) / 1048576.0;

    calibration.par_p2 = ((double)par_p2 - 16384.0) / 536870912.0;

    calibration.par_p3 = (double)par_p3 / 4294967296.0;

    calibration.par_p4 = (double)par_p4 / 137438953472.0;

    calibration.par_p5 = (double)par_p5 * 8.0;

    calibration.par_p6 = (double)par_p6 / 64.0;

    calibration.par_p7 = (double)par_p7 / 256.0;

    calibration.par_p8 = (double)par_p8 / 32768.0;

    calibration.par_p9 = (double)par_p9 / 281474976710656.0;

    calibration.par_p10 = (double)par_p10 / 281474976710656.0;

    calibration.par_p11 = (double)par_p11 / 36893488147419103232.0;

    calibrationLoaded = true;
}

// temperature compensation
double compensateTemperature(uint32_t rawTemp) {
    double deltaT = (double)rawTemp - calibration.par_t1;
    double linear = deltaT* calibration.par_t2;
    double quad = deltaT*deltaT * calibration.par_t3;
    double temp = linear + quad;

    return temp;
}

// pressure compensation

double compensatePressure(uint32_t rawPressure, double tLin) {

    double p = (double)rawPressure;

    // component A: Temperature-dependent offset
    double A =
        calibration.par_p5 +
        calibration.par_p6 * tLin +
        calibration.par_p7 * tLin * tLin +
        calibration.par_p8 * tLin * tLin * tLin;

    // component B: Linear pressure contribution
    double B = p * (
        calibration.par_p1 +
        calibration.par_p2 * tLin +
        calibration.par_p3 * tLin * tLin +
        calibration.par_p4 * tLin * tLin * tLin
    );

    // component C: Non-linear pressure contribution
    double C =
        p * p * (calibration.par_p9 + calibration.par_p10 * tLin) +
        p * p * p * calibration.par_p11;

    // Final compensated pressure in Pa
    double pressure = A + B + C;

    return pressure;
}

BMPData readBMPData() {

    if (!calibrationLoaded) {
        return {NAN, NAN};
    }

    uint32_t rawTemp;
    uint32_t rawPres;

    if (!readRawMeasurements(rawPres, rawTemp)) {
        return {NAN, NAN};
    }

    double temperature = compensateTemperature(rawTemp);
    double pressure = compensatePressure(rawPres, temperature);

    return {pressure, temperature};
}