#include <WiFiClientSecure.h>
#include "http.h"
#include "main.h"

#define PLANT_INFO_FIELD_SIZE 32
#define MAX_URL_IMAGE_LENGTH 512

enum BOT_COMMAND {
    HELP,
    START,
    RESET,
    WATER_REFILLED,
    WATER_REMAINING,
    SOIL_MOISTURE,
    TEMPERATURE,
    HUMIDITY,
    INVALID
  };

  struct plantInfo{
    char name[PLANT_INFO_FIELD_SIZE];
    char pid[PLANT_INFO_FIELD_SIZE];
    char image[MAX_URL_IMAGE_LENGTH];
  };

class Bot{
  public:
    Bot();
    void begin();
    void update();
    bool isSetupDone();
    void notifyWaterAlmostFinished();
    void checkHumAndTemp();

  private:
  bool setupDone;
  unsigned long lastCheck;
  UniversalTelegramBot* bot;
  WiFiClientSecure telegramClient;
  HTTPRequest* http;
  
  void handleMessage(telegramMessage message);
  BOT_COMMAND stringCommandToEnum(String command);
  plantInfo* queryUserSetup(String chatId);
  String waitForNextMessage();
  void sendPlantListToUser(const String& query, JsonArray results, String chatId, int count);
  void updateParameters(plantInfo* plant, String chatId);
};

