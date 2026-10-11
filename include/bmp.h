#pragma once
#include <Arduino.h>

void initializeBMP();

bool isBMPConnected();

uint32_t readPressure();

uint32_t readTemperature();
