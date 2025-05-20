#include <DHT.h>
#include "sensors.h"
#include "main.h"

// soil moisture sensor
#define SOIL_MOISTURE_PIN 35
#define DRY_SENSOR 3000
#define WET_SENSOR 1000
#define READINGS 10

// 5v relay for pump
#define RELAY_WATER_PUMP 14
#define MS_ACTIVE_PUMP 3000

// DHT sensor
#define DHT_PIN 25
#define DHTTYPE DHT22

DHT dht(DHT_PIN, DHTTYPE);

// ultrasonic sensor
#define ULTRA_TRIG_PIN 16
#define ULTRA_ECHO_PIN 17

//bucket and pump infos
#define SENSOR_HEIGHT 11.75

//speed of sound in centimeters per microsecond;
#define SPEED_SOUND_CMMS 0.0343


float getWaterRemainingInCm() {

  // Clears the trigPin
  digitalWrite(ULTRA_TRIG_PIN, LOW);  
	delayMicroseconds(2);  

   // Sets the trigPin on HIGH state for 10 micro seconds
	digitalWrite(ULTRA_TRIG_PIN, HIGH);  
	delayMicroseconds(10);  
	digitalWrite(ULTRA_TRIG_PIN, LOW); 

 // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(ULTRA_ECHO_PIN, HIGH);  

  // divided by two because sound waves travel to object and back
  float distanceFromWater = (duration * SPEED_SOUND_CMMS)/2;
  return SENSOR_HEIGHT - distanceFromWater;
}

int getSoilMoisture() {
  int sumSoilMoistureVal = 0;
  int i = 0;
  while(i++ < READINGS){
    sumSoilMoistureVal += analogRead(SOIL_MOISTURE_PIN);
  }
  int soilMoistureVal = sumSoilMoistureVal/READINGS;
  return map(soilMoistureVal, WET_SENSOR, DRY_SENSOR, 100, 0);
}

void sensorsInit() {
    // ultrasonic - distance
  pinMode(ULTRA_TRIG_PIN, OUTPUT);
  pinMode(ULTRA_ECHO_PIN, INPUT);

  //relay - water pumpt
  pinMode(RELAY_WATER_PUMP, OUTPUT);
  //set relay to high to deactivate pump. It works in reverse way due to NPN transistor 
  digitalWrite(RELAY_WATER_PUMP, HIGH);

  //init dht sensor
  dht.begin();
}

void activatePump(){
  digitalWrite(RELAY_WATER_PUMP, LOW);
  delay(MS_ACTIVE_PUMP);
  digitalWrite(RELAY_WATER_PUMP, HIGH);
}

bool isBucketToRefill(){
  return getWaterRemainingInCm() < float(MIN_WATER_FOR_PUMP);
}


bool needToIrrigate(){
  int percentageHum = getSoilMoisture();
  return percentageHum < ((maxPlantSoilMoisture - minPlantSoilMoisture)/2 + minPlantSoilMoisture);
}

float getTemperature(){
  return dht.readTemperature();
}

float getHumidity(){
  return dht.readHumidity();
}

bool isTemperatureOk(){
  float temp = dht.readTemperature();
  return (temp < minPlantTemperature || temp > maxPlantTemperature);
}

bool isHumidityOk(){
  float hum = dht.readHumidity();
  return (hum < minPlantHumidity || hum > maxPlantHumidity);
}