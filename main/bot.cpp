#include "secrets.h"
#include "bot.h"
#include "sensors.h"


//check message every second
#define BOT_REQUEST_DELAY 1000

#define MIN_LENGTH_PLANT_NAME 3

#define COMMAND_COUNT 8

struct CommandInfo {
  const char* command;
  const char* description;
};

const CommandInfo commandList[] = {
  {"/help",  ""},
  {"/start", "Use this command to configure the watering system. If it's already configured, you need to use /reset first."},
  {"/reset", "Use this command to reset the watering system."},
  {"/waterrefilled", "Use this command to notify that water in bucket is refilled. This is useful to unlock watering system, when it goes off due to lack of water."},
  {"/waterremaining", "Use this command to get the water remaining (in cm). Water system goes down when water remaining is less then 4cm."},
  {"/soilmoisture", "Use this command to get the soil moisture percentage."},
  {"/temperature", "Use this command to get temperature."},
  {"/humidity", "Use this command to get humidity."}
};


bool isPlantNameValid(String plantName, String chatId){
  if (plantName.length() > MIN_LENGTH_PLANT_NAME) return true;
  return false;
}

String createMessageFromQueryResults(String searchedPlant, int totalResults, JsonArray results){
  int resultsSize = results.size();
  String message = "The search '"+ searchedPlant+ "' produces " + String(totalResults) + " results.\nHere are listed " + String(resultsSize)+" plants:"; 
  for (int i = 0; i<resultsSize; i++){
    message += "\n" + String(i+1) + ". " + String(results[i]["alias"]);
  }
  message += "\nPick one sending a message containing the index (1,2,3,...) or a string to refine the search.";
  return message;
}

String getHelpMessage() {
  String help = "Available commands:\n\n";
  for (size_t i = 0; i < COMMAND_COUNT; i++) {
    help += String(commandList[i].command) + " - " + commandList[i].description + "\n";
  }
  return help;
}




void Bot::begin(){
  telegramClient.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  bot = new UniversalTelegramBot(BOT_TOKEN, telegramClient);
  http = new HTTPRequest();
  http -> begin();
  
  bot->sendMessage(CHAT_ID, "To setup watering system, send command /start");
}

void Bot::update(){
  if (millis() > lastCheck + BOT_REQUEST_DELAY)  {
    // handle message from telegram
    numNewMessages = bot->getUpdates(bot->last_message_received+1);
    for(int i = 0; i < numNewMessages; i++){
      handleMessage(bot->messages[i]);
    }
    numNewMessages = 0;
    lastCheck = millis();
  }
}

void Bot::handleMessage(telegramMessage message){
  String answer = "";

  if (String(message.chat_id) != CHAT_ID){
    answer =  "Unauthorized user";
    bot->sendMessage(message.chat_id, answer, "");
    return;
  }

  BOT_COMMAND cmd = stringCommandToEnum(message.text);

  switch (cmd) {
    case HELP: {
      bot->sendMessage(message.chat_id, getHelpMessage().c_str(), "");
      break;
    }
    case START:{
      if (setupDone) 
        bot->sendMessage(message.chat_id, "Watering system already configured. If you want to reset, use commnad /reset", "");
      else {
        plantInfo* plant = queryUserSetup(message.chat_id);
        updateParameters(plant, message.chat_id);
      }
      break;
    }
    case RESET:{
      reset();
      bot->sendMessage(message.chat_id, "Watering system reset done. Restart configuration using command /start.", "");
      break;
    }
    case WATER_REFILLED:{
      if (isBucketToRefill()){
        bot->sendMessage(message.chat_id, "Not enough water to activate irrigation.", "");
      } else {
        bot->sendMessage(message.chat_id, "Watering system activated.", "");
        stopIrrigation = false;
      }
      break;
    }
    case WATER_REMAINING:{
      float waterInCm = getWaterRemainingInCm();
      bot->sendMessage(message.chat_id, ("Water remaining in bucket is " + String(waterInCm) + " cm. The system goes down if water goes under " + String(MIN_WATER_FOR_PUMP) +" cm.").c_str(), "");
      break;
    }
    case SOIL_MOISTURE:{
      int percentageSoilMoisture = getSoilMoisture();
      bot->sendMessage(message.chat_id, ("Soil moisture percentage is " + String(percentageSoilMoisture) + " %.\nThe plant you are growing is " + plantName + ".\nSoil moisture value is preferred to be between " + String(minPlantSoilMoisture) + " and " + String(maxPlantSoilMoisture) + ".").c_str(), "");
      break;
    }
    case TEMPERATURE:{
      float temp = getTemperature();
      bot->sendMessage(message.chat_id, ("Temperature is " + String(temp) + "C .\nThe plant you are growing is " + plantName + ".\nTemperature value is preferred to be between " + String(minPlantTemperature) + " and " + String(maxPlantTemperature) + ".").c_str(), "");
      break;
    }
    case HUMIDITY:{
      float hum = getHumidity();
      bot->sendMessage(message.chat_id, ("Humidity is " + String(hum) + " %.\nThe plant you are growing is " + plantName + ".\Humidity value is preferred to be between " + String(minPlantHumidity) + " and " + String(maxPlantHumidity) + ".").c_str(), "");
      break;
    }
    case INVALID:{
      bot->sendMessage(message.chat_id, "No action found. If you want to have a list of all available commands, use \help.", "");
      break;
    }
  }
}

String Bot::waitForNextMessage() {
  int numNewMessages = 0;
  while (!(numNewMessages = bot->getUpdates(bot->last_message_received + 1))) {
    delay(500);
  }
  return String(bot->messages[0].text);
}

BOT_COMMAND Bot::stringCommandToEnum(String command){
  command.toLowerCase();
  if(!setupDone && command != commandList[START].command) return INVALID;
  for(int i = 0; i < COMMAND_COUNT; i++){
    if (command == commandList[i].command) return (BOT_COMMAND)i;
  }
  return INVALID;
}

plantInfo* Bot::queryUserSetup(String chatId){
  String plantToSearch = "";
  bool waitingResFromList = false;
  int numNewMessages = 0;
  long indexChosen = 0;
  StaticJsonDocument<1024> doc;

  plantInfo* plant = new plantInfo();
  strcpy(plant->name, "");
  strcpy(plant->pid, "");
  strcpy(plant->image, "");

  bot->sendMessage(chatId, "Welcome to watering System setup. Digit what plant do you wanna grow. \nIt's better to use scientific name.");

  //name not already init
  while (strcmp(plant->name, "") == 0){

    plantToSearch = waitForNextMessage();

    // User digit a number from list of plant of the request made before.
    if (waitingResFromList && (indexChosen = plantToSearch.toInt()) != 0){
      JsonArray results = doc["results"].as<JsonArray>();
      if(indexChosen - 1 >= results.size() || indexChosen - 1 < 0){
        String message = "Invalid index. Pick one plant sending a message containing the index (1,2,3) from list or a string to refine the search.";
        bot->sendMessage(chatId, message, "");
        continue;
      }
      strcpy(plant->pid, results[indexChosen-1]["pid"]);
      strcpy(plant->name, results[indexChosen-1]["display_pid"]);
      break;
    }

    if(!isPlantNameValid(plantToSearch, chatId)) {
      bot->sendMessage(chatId, "Plant name length must have at least 3 letters.");
      continue;
    }

    plantToSearch = http->encodeHttpString(plantToSearch);

    String uri = BASE_URL_API_OPENPLANT + URL_SEARCH_API_OPENPLANT + "?alias=" + plantToSearch + "&limit=10", chatId;

    String payload = http->requestOpenPlant(uri);
    if (payload == "") break;

    doc = http -> deserializePayload(payload, bot, chatId);
    if (doc.isNull()) continue;

    int count = doc["count"];

    if (count == 0) {
      bot->sendMessage(chatId, "No plant found. Try again.");
      continue;
    }

    JsonArray results = doc["results"].as<JsonArray>();

    // done if only one result. Can exit from while.
    if(count == 1){
      strcpy(plant->pid, results[0]["pid"]);
      strcpy(plant->name, results[0]["display_pid"]);
      break;
    }

    // if code arrives here, we got a list of plants. User can select one.
    sendPlantListToUser(plantToSearch, doc["results"].as<JsonArray>(), chatId, count);
    waitingResFromList = true;
  }

  
  String message = "You pick '" + String(plant->name) +"'. Now the watering system will adjust parameter based on this plant.";
  bot->sendMessage(chatId, message, "");
  return plant;
}

void Bot::sendPlantListToUser(const String& query, JsonArray results, String chatId, int count) {
  String message = createMessageFromQueryResults(query, count, results);
  bot->sendMessage(chatId, message.c_str());
}

void Bot::updateParameters(plantInfo* plant, String chatId){

  String pathQueryPlant = String(plant->pid);
  //pathQueryPlant.replace(" ", "%20");

  String URL = BASE_URL_API_OPENPLANT + URL_DETAIL_API_OPENPLANT + "/" + pathQueryPlant + "/";

  String payload = http->requestOpenPlant(URL);

  if(payload == "") {
    String message = "Error querying system to update parameters. Retry later using /start.";
    bot->sendMessage(chatId, message.c_str());
    return;
  }

  Serial.println("GET REQUEST update parameters done");

  StaticJsonDocument<1024> doc = http->deserializePayload(payload, bot, chatId);

  Serial.println("JSONDESERIALIZED update parameters done");

  // init variables
  maxPlantSoilMoisture = doc["max_soil_moist"];
  minPlantSoilMoisture = doc["min_soil_moist"];
  minPlantTemperature = doc["min_temp"];
  maxPlantTemperature = doc["max_temp"];
  minPlantHumidity = doc["min_env_humid"];
  maxPlantHumidity = doc["max_env_humid"];
  plantName = String(doc["display_pid"]);
  strcpy(plant->image, doc["image_url"]);

  bot->sendMessage(chatId, ("Setup done.\nPlant:" + plantName + 
    ".\nMinimum soil moisture percentage:" + String(minPlantSoilMoisture)+ 
    ".\nMaximum soil moisture percentage:" + String(maxPlantSoilMoisture) +
    ".\nMinimum environmental humidity:" + String(minPlantHumidity)+ 
    ".\nMaximum environmental humidity:" + String(maxPlantHumidity)+ 
    ".\nMinimum environmental temperature:" + String(minPlantTemperature) + 
    ".\nMaximum environmental temperature:" + String(maxPlantTemperature)).c_str(), "");
  if (strcmp(plant->image, "") != 0){
    bot->sendPhoto(chatId, plant->image);
  }
  
  return;
}

bool Bot::isSetupDone(){
  return setupDone;
}

void Bot::notifyWaterAlmostFinished(){
  bot->sendMessage(CHAT_ID, "The water is almost out! There will be no automatic watering. To re-enable it, fill the bucket with water and use the /waterrefilled command.", "");
}

void Bot::checkHumAndTemp(){
  if(!setupDone) return;
  if (!isTemperatureOk())
    bot->sendMessage(CHAT_ID, String("Current temperature is:" + String(getTemperature()) + "C. Recommended temperature is between " + String(minPlantTemperature) + " and " + String(maxPlantTemperature) + ". If you can, move the plant to another place.").c_str(), "");
  if (!isHumidityOk()){
    bot->sendMessage(CHAT_ID, String("Current humidity is:" + String(getHumidity()) + "%. Recommended humidity is between " + String(minPlantHumidity) + " and " + String(maxPlantHumidity) + ". If you can, move the plant to another place.").c_str(), "");
  }
}