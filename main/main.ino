#include <UniversalTelegramBot.h>
#include <CronAlarms.h>
#include "sensors.h"
#include "main.h"
#include "bot.h"
#include "secrets.h"

CronId id;
Bot* bot;

void checkHumAndTempWrapper() {
    if (bot != nullptr) {
        bot->checkHumAndTemp();
    }
}


void setup(){
  Serial.begin(115200);

  delay(1000);

  Serial.println("Done. Connect to Wifi");

  // connect to wifi
  WiFi.begin(SSID_WIFI, PASS_WIFI);
  Serial.println("\nConnecting");

  while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(300);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  Serial.println("initialize sensors");

  sensorsInit();

  Serial.println("Done. Initialize consts");
  reset();
  Serial.println("Done. Init Bot");
  bot = new Bot();
  bot->begin();
  Serial.println("Done.");

  //Serial.println("Create Cron");

  id = Cron.create("0 0 12 * * *", checkHumAndTempWrapper, false);
}

/*** LOOP ***/

void loop() {
  if (bot->isSetupDone()){
    //check if bucket is almost empty. stopIrrigation per entrare solo una volta nell'if
    if(isBucketToRefill() && !stopIrrigation){
      bot->notifyWaterAlmostFinished();
      stopIrrigation = true;
    }

    // check humidity of soil and water availability. If so, water the plant
    if(needToIrrigate() && !stopIrrigation){
      activatePump();
      delay(300);
    }
  }

  if (WiFi.status() == WL_CONNECTED){
    bot->update();
  }
  delay(1000);
}








