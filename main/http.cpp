#include <http.h>
#include <HTTPClient.h>
#include "secrets.h"

const char* OPENPLANT_CERTIFICATE_ROOT = 
"-----BEGIN CERTIFICATE-----\n" \
"MIICCTCCAY6gAwIBAgINAgPlwGjvYxqccpBQUjAKBggqhkjOPQQDAzBHMQswCQYD\n" \
"VQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEUMBIG\n" \
"A1UEAxMLR1RTIFJvb3QgUjQwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAwMDAw\n" \
"WjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2Vz\n" \
"IExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjQwdjAQBgcqhkjOPQIBBgUrgQQAIgNi\n" \
"AATzdHOnaItgrkO4NcWBMHtLSZ37wWHO5t5GvWvVYRg1rkDdc/eJkTBa6zzuhXyi\n" \
"QHY7qca4R9gq55KRanPpsXI5nymfopjTX15YhmUPoYRlBtHci8nHc8iMai/lxKvR\n" \
"HYqjQjBAMA4GA1UdDwEB/wQEAwIBhjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQW\n" \
"BBSATNbrdP9JNqPV2Py1PsVq8JQdjDAKBggqhkjOPQQDAwNpADBmAjEA6ED/g94D\n" \
"9J+uHXqnLrmvT/aDHQ4thQEd0dlq7A/Cr8deVl5c1RxYIigL9zC2L7F8AjEA8GE8\n" \
"p/SgguMh1YQdc4acLa/KNJvxn7kjNuK8YAOdgLOaVsjh4rsUecrNIdSUtUlD\n" \
"-----END CERTIFICATE-----\n";


String HTTPRequest::encodeHttpString(String input) {
  String output = "";
  for (unsigned int i = 0; i < input.length(); i++) {
    char c = input.charAt(i);
    if (c == ' ') {
      output += "%20";
    } else {
      output += c;
    }
  }
  return output;
}

void HTTPRequest::begin(){
  openPlantClient.setCACert(OPENPLANT_CERTIFICATE_ROOT);
}

String HTTPRequest::requestOpenPlant(String URL){
  HTTPClient https;
  https.begin(openPlantClient, URL.c_str());
  https.addHeader("Authorization", (String("Token ") + OPENPLANT_API_KEY).c_str());
  int httpCode = https.GET();
  if (httpCode <= 0 || !(httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)){
    https.end();
    return "";
  }
  
  String payload = https.getString();
  https.end();
  return payload;
}

StaticJsonDocument<1024> HTTPRequest::deserializePayload(String payload, UniversalTelegramBot* bot, String chatId){
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    String message =  String("Error deserializing JSON response. Retry later.\nError:") + error.c_str();
    bot->sendMessage(chatId, message.c_str());
  }

  return doc;
}