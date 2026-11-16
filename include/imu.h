#pragma once
#include <Arduino.h>



struct IMUData {
    float ax;
    float ay;
    float az;

    float gx;
    float gy;
    float gz;
};

void initializeIMU();

bool isIMUConnected();

IMUData readIMUData();


