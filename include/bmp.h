#pragma once
#include <Arduino.h>



struct BMPData{
    double pressure;
    double temperature;
};
void initializeBMP();

bool isBMPConnected();

uint32_t readPressure();

uint32_t readTemperature();


BMPData readBMPData();