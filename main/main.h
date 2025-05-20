#ifndef MAIN_H
#define MAIN_H

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <WiFi.h>

extern bool stopIrrigation;
extern int minPlantSoilMoisture;
extern int maxPlantSoilMoisture;
extern int minPlantTemperature;
extern int maxPlantTemperature;
extern int minPlantHumidity;
extern int maxPlantHumidity;
extern String plantName;


void reset();

#endif