#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "index.h"

ESP8266WebServer server(80);

const char* ssid     = "DoorLock";
const char* password = "RAPtor1234";

String newssid = "ssid";
String newpass = "";

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("HotSpt IP:");
  Serial.println(myIP);

  server.on("/", handleIndex);
  server.on("/save", handleSave);
  server.begin();
}

void loop() {
  server.handleClient();
}

void handleIndex() {
  Serial.println("index");
  String s = MAIN_page;
  s.replace("@@ssid@@", newssid);
  s.replace("@@pass@@", newpass);
  server.send(200, "text/html", s);
  return;
}

void handleSave() {
   Serial.println("saving");
  if (server.hasArg("ssid")== false) {
    server.send(200, "text/plain", "ssid not received");
    Serial.println("Body not received");
    return;
  }

  if (server.hasArg("password")== false) {
    server.send(200, "text/plain", "sspasswordid not received");
    Serial.println("Body not received");
    return;
  }
  
  newssid = server.arg("ssid");
  Serial.println(newssid);
  newpass = server.arg("password");
  Serial.println(newpass);
  
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "");
  connectWifi();
  return;
}

void connectWifi() {
  
  WiFi.begin(newssid.c_str(), newpass.c_str());
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

