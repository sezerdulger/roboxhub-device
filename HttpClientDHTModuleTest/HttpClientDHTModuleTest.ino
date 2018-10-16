#include <Adafruit_Sensor.h>

#include <DHT.h>
#include <DHT_U.h>

/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;

#define DHTPIN            D4
#define DHTTYPE           DHT11

DHT_Unified dht4(D4, DHTTYPE);

uint32_t delayMS;

ESP8266WebServer server(80);

const char* ssid     = "Sezer";
const char* password = "RAPtor1234";
String host = "";
int port = 0;
String url = "";
String exacturl = "";

IPAddress broadcastIp(192, 168, 1, 255);
const int udpPort = 5001;

WiFiClient wificlient;
WiFiUDP udp;

extern "C" {
  uint16 readvdd33(void);
}

ADC_MODE(ADC_VCC);

void setup() {

  Serial.begin(115200);
  dht4.begin();
  /*dht1.begin();
  dht2.begin();
  dht3.begin();
  dht4.begin();
  dht5.begin();
  dht6.begin();
  dht7.begin();
  dht8.begin();
  dht9.begin();
  dht10.begin();*/
  
  delay(10);
  
  sensor_t sensor;
  dht4.temperature().getSensor(&sensor);
   Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");  
  Serial.println("------------------------------------");
  // Print humidity sensor details.
  dht4.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println("------------------------------------");
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/body", handleBody);
  server.begin();
}

void loop() {
 sendData(dht4, "dht4");
 /*sendData(dht1, "dht1");
 sendData(dht2, "dht2");
 sendData(dht3, "dht3");
 sendData(dht4, "dht4");
 sendData(dht5, "dht5");
 sendData(dht6, "dht6");
 sendData(dht7, "dht7");
 sendData(dht8, "dht8");
 sendData(dht9, "dht9");
 sendData(dht10, "dht10");*/
 
 
  delay(10000);
}

void sendData(DHT_Unified dht, String dht_id) {
 uint16 batteryStatus = readvdd33();
  float temperature;
  float humidity;
  Serial.println("batteryStatus ");
  Serial.println(batteryStatus);
  Serial.println("batteryStatus2 ");
  Serial.println(ESP.getVcc());
    /*********************************************/
    // Delay between measurements.
  delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event;  
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  else {
    Serial.print("Temperature: ");
    Serial.print(event.temperature);
    Serial.println(" *C");
    temperature=event.temperature;
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
    Serial.print("Humidity: ");
    Serial.print(event.relative_humidity);
    Serial.println("%");
    humidity=event.relative_humidity;
  }
  /**************************************************/
  host="test";
  //server.handleClient();
  if (host != "") {
    Serial.print("Trying to send...host is ");
    Serial.println(host);
    String message="{\"data\": 2}";
    
    HTTPClient http;
    http.begin("http://roboxhub.com:32731/api/1.0/deviceData");
    //http.begin(exacturl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Accept", "application/json");
    http.addHeader("Authorization", "Basic ZXJoYW46MTIzNDU2Nzg=");
    http.addHeader("Content-Length", String(message.length()));

    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["name"] = "erhan";
    root["dht_id"]=dht_id;
    root["data"] = jsonBuffer.createObject();
    root["data"]["temp"]=temperature;
    root["data"]["humidity"]=humidity;
    root["data"]["device_id"]="erhan";
    root["data"]["battery"]=batteryStatus;
    root["data"]["battery2"]=ESP.getVcc();

    String jsonString;
    root.printTo(jsonString);
    Serial.println(jsonString);
    int httpCode = http.POST(jsonString);
    Serial.print("httpCode: ");
    Serial.println(httpCode);
    // Get the request response payload
    String payload = http.getString();
    Serial.println(payload);
    
    http.end();
  }
  else {
    udp.beginPacket(broadcastIp, udpPort);
    udp.write("hi");
    udp.endPacket();
  }
}

void handleBody() {
  Serial.println("Received something");
  if (server.hasArg("plain")== false){
    server.send(200, "text/plain", "Body not received");
    Serial.println("Body not received");
    return;
  }
  
  String message = server.arg("plain");
  
  const size_t bufferSize = JSON_OBJECT_SIZE(1) + 50;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root = jsonBuffer.parseObject(message);
  host = String(root["host"].as<char*>());
  port = root["port"];
  url = String(root["url"].as<char*>());
  exacturl = String(root["exacturl"].as<char*>());
  
  server.send(200, "text/plain", "{'status': 1}");
  Serial.println(message);
  server.stop();
}

