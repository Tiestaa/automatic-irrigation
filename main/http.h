#include "main.h"
#define BASE_URL_API_OPENPLANT String("https://open.plantbook.io/api/v1/plant")
#define URL_SEARCH_API_OPENPLANT String("/search")
#define URL_DETAIL_API_OPENPLANT String("/detail")


class HTTPRequest{
  public:
  HTTPRequest();
  String encodeHttpString(String input);
  String requestOpenPlant(String URL);
  StaticJsonDocument<1024> deserializePayload(String payload,UniversalTelegramBot* bot, String chatId);

  private:
  WiFiClientSecure openPlantClient; 
};

