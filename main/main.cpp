#include "main.h"
bool stopIrrigation = false;
String plantName = "";
int minPlantSoilMoisture = 0;
int maxPlantSoilMoisture = 0;
int minPlantTemperature = 0;
int maxPlantTemperature = 0;
int minPlantHumidity = 0;
int maxPlantHumidity = 0;

void reset(){
  stopIrrigation = false;
  plantName = "";
  minPlantSoilMoisture = 0;
  maxPlantSoilMoisture = 0;
  minPlantTemperature = 0;
  maxPlantTemperature = 0;
  minPlantHumidity = 0;
  maxPlantHumidity = 0;
}