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

ESP8266WebServer server(80);

const char* ssid     = "Sezer";
const char* password = "RAPtor1234";
const char* mqttServer = "192.168.1.21";
const int mqttPort = 1883;
const char* mqttUser = "smarthome";
const char* mqttPassword = "smartpassword";
const char* host = "192.168.1.21";

IPAddress broadcastIp(192, 168, 1, 255);
const int udpPort = 5001;

WiFiClient wificlient;
PubSubClient client(wificlient);
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

  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
 
  
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
 
      Serial.println("connected");
      
      client.subscribe("esp/test");

      server.on("/body", handleBody);
      server.begin();
     
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
}

void loop() {
  delay(2000);
  server.handleClient();
  
  if (!client.publish("esp/test", "Hello from ESP8266")) {
    Serial.println("cant sent");  
  }
  Serial.println("sent");
  udp.beginPacket(broadcastIp, udpPort);
  udp.write("hi");
  udp.endPacket();

}

void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 
  Serial.println();
  Serial.println("-----------------------");
 
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

