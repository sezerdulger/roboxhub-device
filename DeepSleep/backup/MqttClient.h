#ifndef MqttClient_h
#define MqttClient_h
#include "Arduino.h"

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
class MqttClient
{
  public:
    MqttClient(WiFiClient *wificlient);
    bool connect(String exchange, String user, String password);
    void setServer(String mqttServer, int mqttPort);
    bool publish(String exchange, String message);
    bool connected();
    void subscribe(String exchange);
    void setCallback(MQTT_CALLBACK_SIGNATURE);
    int state();
  private:
    WiFiClient* _wificlient;
    PubSubClient _mqttClient;
    
    String _mqttServer;
    String _mqttPort;
};

#endif
