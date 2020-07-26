#include "Updater.h"
#include <ESP8266httpUpdate.h>
#include "Arduino.h"

Updater::Updater(String url)
{
  _url = url;
}

void Updater::beginUpdate()
{
  ESPhttpUpdate.rebootOnUpdate(true);
  t_httpUpdate_return ret = ESPhttpUpdate.update(_url);
  
  switch(ret) {
   case HTTP_UPDATE_FAILED:
       Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
       //logMqtt(String(ESPhttpUpdate.getLastError()));
       //logMqtt(ESPhttpUpdate.getLastErrorString());
       Serial.println(String(ESPhttpUpdate.getLastError()));
       Serial.println(ESPhttpUpdate.getLastErrorString());
       break;

   case HTTP_UPDATE_NO_UPDATES:
       Serial.println("HTTP_UPDATE_NO_UPDATES");
       //logMqtt("HTTP_UPDATE_NO_UPDATES");
       break;

   case HTTP_UPDATE_OK:
       Serial.println("HTTP_UPDATE_OK");
       //logMqtt("HTTP_UPDATE_OK");
       break;

   default:
      Serial.println("Undefined HTTP_UPDATE Code:");
      Serial.println(String(ret));
       //logMqtt("Undefined HTTP_UPDATE Code: ");logMqtt(String(ret));
  }
  
  //ESP.restart(); 
}

