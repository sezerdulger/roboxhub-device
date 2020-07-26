#include "MqttClient.h"
#include "Arduino.h"

#include <WiFiClientSecure.h>
#include <PubSubClient.h>

MqttClient::MqttClient(WiFiClient *wificlient)
{
  _wificlient = wificlient;
  _mqttClient(_wificlient);
}

void MqttClient::setServer(String mqttServer, int mqttPort)
{
  _mqttServer = mqttServer;
  _mqttPort = _mqttPort;
}

bool MqttClient::connected()
{
  return _mqttClient.connected();
}

bool MqttClient::connect(String exchange, String user, String pass)
{
  return _mqttClient.connect(exchange, user, pass);
}

bool MqttClient::publish(String exchange, String message)
{
  return _mqttClient.publish(exchange, message.c_str());
}

void MqttClient::subscribe(String exchange)
{
  return _mqttClient.subscribe(exchange, message.c_str());
}

void MqttClient::setCallback(MQTT_CALLBACK_SIGNATURE)
{
  return _mqttClient.setCallback(callback);
}

int MqttClient::state()
{
  return _mqttClient.state();
}
