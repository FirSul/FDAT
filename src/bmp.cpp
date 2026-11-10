
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

// bmp init
void initializeBMP() {
    Wire.begin(8, 9); // SDA = 8, SCL = 9

    Wire.beginTransmission(BMP_ADDRESS);
    Wire.write(0x1B); // PWR_CTRL register
    Wire.write(0x33); // Enable pressure, temperature, normal mode
    Wire.endTransmission();

    readCalibration();
}

bool isBMPConnected() {
    Wire.beginTransmission(BMP_ADDRESS);
    return Wire.endTransmission() == 0;
}

// Reading register func:
static uint32_t readRaw24(uint8_t startRegister) {
    Wire.beginTransmission(BMP_ADDRESS);
    Wire.write(startRegister);
    Wire.endTransmission(false);

    Wire.requestFrom(BMP_ADDRESS, 3);

    uint8_t xlsb = Wire.read();
    uint8_t lsb = Wire.read();
    uint8_t msb = Wire.read();

    uint32_t rawvalue =
        ((uint32_t)msb << 16) |
        ((uint32_t)lsb << 8) |
        xlsb;

    return rawvalue;
}

// Raw values from pres and temp
uint32_t readPressure() {
    return readRaw24(0x04);
}

uint32_t readTemperature() {
    return readRaw24(0x07);
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

    // Component A: Temperature-dependent offset
    double A =
        calibration.par_p5 +
        calibration.par_p6 * tLin +
        calibration.par_p7 * tLin * tLin +
        calibration.par_p8 * tLin * tLin * tLin;

    // Component B: Linear pressure contribution
    double B = p * (
        calibration.par_p1 +
        calibration.par_p2 * tLin +
        calibration.par_p3 * tLin * tLin +
        calibration.par_p4 * tLin * tLin * tLin
    );

    // Component C: Nonlinear pressure contribution
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

    uint32_t rawTemp = readTemperature();
    uint32_t rawPres = readPressure();

    double temperature = compensateTemperature(rawTemp);
    double pressure = compensatePressure(rawPres, temperature);

    return {pressure, temperature};
}