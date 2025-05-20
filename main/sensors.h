#pragma once

#define MIN_WATER_FOR_PUMP 2.5

void sensorsInit();

int getSoilMoisture();
float getHumidity();
float getTemperature();
float getWaterRemainingInCm();

bool isBucketToRefill();
bool needToIrrigate();
void activatePump();
bool isTemperatureOk();
bool isHumidityOk();
