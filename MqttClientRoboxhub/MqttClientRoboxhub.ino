/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>

#include <ESP.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

ESP8266WebServer server(80);

const char* ssid     = "Sezer";
const char* password = "RAPtor1234";
const char* mqttServer = "192.168.34.10";
const int mqttPort = 30769;
const char* mqttUser = "guest";
const char* mqttPassword = "guest";
const char* host = "192.168.1.21";
uint32_t delayMS;
uint32_t delayTime=10000;

IPAddress broadcastIp(192, 168, 1, 255);
const int udpPort = 5001;

WiFiClient wificlient;
PubSubClient client(wificlient);
WiFiUDP udp;

extern "C" {
  uint16 readvdd33(void);
}

float lastTemperature = 0;
float lastHumidity = 0;

ADC_MODE(ADC_VCC);

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

DHT_Unified dht4(DHTPIN, DHTTYPE);

String device_sezer="sezer";
String token_sezer="Basic c2V6ZXI6MTIzNDU2Nzg=";
String device_erhan="erhan";
String token_erhan = "Basic ZXJoYW46MTIzNDU2Nzg=";

String device_name=device_erhan;
String token=token_erhan;

String data_publish_topic="device.sezer";

void setup() {
  Serial.begin(115200);
  delay(10);
  dht4.begin();
  sensor_t sensor;
  dht4.temperature().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;
  Serial.println("delayMS");
  Serial.println(delayMS);
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

  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
 
      Serial.println("connected");
      
      client.subscribe("esp.test");

      server.on("/body", handleBody);
      server.begin();
     
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
  client.loop();
  sendData(dht4, "dht4");
  delay(delayTime);
 /*
  udp.beginPacket(broadcastIp, udpPort);
  udp.write("hi");
  udp.endPacket();*/

}

void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.println("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.println("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
   
  Serial.println();
  Serial.println("-----------------------");
 
}

void sendData(DHT_Unified dht, String dht_id) {
 uint16 batteryStatus = readvdd33();
  float temperature = 0;
  float humidity = 0;
  Serial.println("batteryStatus ");
  Serial.println(batteryStatus);
  Serial.println("batteryStatus2 ");
  Serial.println(ESP.getVcc());
    /*********************************************/
    // Delay between measurements.
  // Get temperature event and print its value.
  sensors_event_t event;  
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
    temperature = 0;
  
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
    humidity = 0;
  }
  else {
    Serial.print("Humidity: ");
    Serial.print(event.relative_humidity);
    Serial.println("%");
    humidity=event.relative_humidity;
  }
  /**************************************************/
   StaticJsonBuffer<300> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["name"] = device_name;
    root["dht_id"]=dht_id;
    root["data"] = jsonBuffer.createObject();
    root["data"]["temp"]=temperature;
    root["data"]["humidity"]=humidity;
    root["data"]["device_id"]=device_name;
    root["data"]["battery"]=batteryStatus;
    root["data"]["battery2"]=ESP.getVcc();

    String jsonString;
    char buffer[300];
    root.printTo(buffer, sizeof(buffer));
    root.printTo(jsonString);
  Serial.println(jsonString);
  if (lastTemperature != temperature || lastHumidity != humidity) {
    lastTemperature=temperature;
      lastHumidity=humidity;
     if (!client.publish(data_publish_topic.c_str(), buffer)) {
      Serial.println("cant sent mqtt, sending http");  
      post(jsonString);
      
     }
     else {

      Serial.println("sent");
     }
  }
  delay(2000);
}

void handleBody() {
  Serial.println("Received something");
  if (server.hasArg("plain")== false){
    server.send(200, "text/plain", "Body not received");
    Serial.println("Body not received");
    return;
  }
  
  String message = "Body received:\n";
  message += server.arg("plain");
  message += "\n";
  
  server.send(200, "text/plain", message);
  Serial.println(message);
  server.stop();
}

void printSensorInfo() {
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
  
  
}

void post(String jsonString) {
  Serial.print("Trying to send...host is ");
    Serial.println(host);
    
    HTTPClient http;
    http.begin("http://192.168.34.10:32731/api/1.0/deviceData");
    //http.begin(exacturl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Accept", "application/json");
    http.addHeader("Authorization", token);
    http.addHeader("Content-Length", String(jsonString.length()));
    
    Serial.println(jsonString);
    int httpCode = http.POST(jsonString);
    Serial.print("httpCode: ");
    Serial.println(httpCode);
    // Get the request response payload
    String payload = http.getString();
    Serial.println(payload);
    
    http.end();
}

