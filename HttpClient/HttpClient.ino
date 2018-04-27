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

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
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
  server.handleClient();
  if (host != "") {
    Serial.print("Trying to send...host is ");
    Serial.println(host);
    String message="{\"data\": 2}";
    
    HTTPClient http;
    //http.begin("http://192.168.1.21:8080/api/1.0/test");
    http.begin(exacturl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Accept", "application/json");
    http.addHeader("Content-Length", String(message.length()));

    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["data"] = 2;

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
  delay(5000);
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

