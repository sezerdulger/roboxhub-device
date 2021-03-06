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
#include <ArduinoOTA.h>
//#include <ESPhttpUpdate.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>
#include <EEPROM.h>

// Required for LIGHT_SLEEP_T delay mode
extern "C" {
#include "user_interface.h"
}

#include "index.h"
#include "FS.h"


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

DHT_Unified dht(DHTPIN, DHTTYPE);

int delayMS = 10000;
int dataTime = 0;
int dataSendInterval = 60000;

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";

const char* ssid     = "";
const char* password = "";

const char* mqttServer = "roboxhub.com";
const int mqttPort = 31883;
const char* mqttUser = "guest";
const char* mqttPassword = "guest";

const char* host = "roboxhub.com";

int port = 0;
String url = "";
String exacturl = "";

//String newssid = "";
//String newpass = "";

String device_name = "sezer";

WiFiClient wificlient;
PubSubClient mqttClient(wificlient);

extern "C" {
  uint16 readvdd33(void);
}

ADC_MODE(ADC_VCC);

float oldTemperature = 0;
float oldHumidity = 0;
bool send_when_change = false;

//void(* resetFunc) (void) = 0; //declare reset function @ address 0

struct {
    String ssid = "";
    String pass = "";
  } data;

void setup() {

  Serial.begin(115200);
  
  pinMode(2, INPUT_PULLDOWN_16);
  
  delay(100);
  dht.begin();

  Serial.println("ADC_VCC integer");
  Serial.println(ADC_VCC);
  
  sensorInfo();
  //OTAConfig();
  
  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_AP_STA);

  WiFi.softAPConfig(IPAddress(192, 168, 5, 1), IPAddress(192, 168, 5, 0), IPAddress(255, 255, 255, 0));
  WiFi.softAP("IsiNem", "RAPtor1234");

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("HotSpt IP:");
  Serial.println(myIP);

  mqttClient.setServer(mqttServer, mqttPort);

  server.on("/", handleIndex);
  server.on("/save", handleSave);
  //server.on("/login", handleLogin);
  server.begin();
  /*
  EEPROM.begin(1024);

  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, 0);
  }

 
  EEPROM.end();
  
  EEPROM.get(0, data);*/
  /*if (wifiSSIDPass != "") {
    int colonIndex = wifiSSIDPass.indexOf(":");
    newssid = wifiSSIDPass.substring(0, colonIndex);
    newpass = wifiSSIDPass.substring(colonIndex);
  }*/
  //newssid=data.ssid;
  //newpass=data.pass;
  Serial.println("data.ssid");
  Serial.println(data.ssid);
  Serial.println("data.pass");
  Serial.println(data.pass);

  
  if (data.ssid == "" && data.pass == "") {
    data.ssid="Sezer";
    data.pass="RAPtor1234";
    afterSave();
  }

  //mqttConnect();
 
}

void loop() {
   mainloop();
}

void mainloop() {
  //ArduinoOTA.handle();
  connectWifi();
  server.handleClient();

  mqttClient.loop();
  dataTime += delayMS;
  if (dataTime == dataSendInterval) {
    sendToExchange();
    dataTime = 0;
  }
  if (mqttClient.connected()) {
    //ESP.deepSleep(30*1000*1000); //30 seconds
    //delay(5000);
    //delay(2000);
  }
  //wifi_set_sleep_type(LIGHT_SLEEP_T);

  delay(delayMS);

  

  /*if (WiFi.status() == WL_CONNECTED && !mqttClient.connected()) {
    mqttConnect();
  }
  */
}

float readTemp() {
  // Get temperature event and print its value.
  sensors_event_t event;

  dht.temperature().getEvent(&event);
  
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  return event.temperature;
}

float readHumidity() {
  sensors_event_t event;
  
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }

  return event.relative_humidity;
}

void sendToExchange() {

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["data"] = jsonBuffer.createObject();
  
  uint16 batteryStatus = readvdd33();
  
  float temperature;
  float humidity;

  
  temperature = readTemp();
  humidity = readHumidity();
  root["data"]["temp"]=temperature;
  root["data"]["humidity"]=humidity;
  /*
  for (int i = 0; i < 6; i++) {
    DHT_Unified dht(i, DHTTYPE);
    temperature = readTemp(dht);
    humidity = readHumidity(dht);
    root["data"]["temp" + i]=temperature;
    root["data"]["humidity" + i]=humidity;
    logMqtt(String(temperature));
    logMqtt(String(humidity));
    delay(1000);
  }

  for (int i = 12; i < 17; i++) {
    DHT_Unified dht(i, DHTTYPE);
    temperature = readTemp(dht);
    humidity = readHumidity(dht);
    root["data"]["temp" + i]=temperature;
    root["data"]["humidity" + i]=humidity;
    logMqtt(String(temperature));
    logMqtt(String(humidity));
    delay(1000);
  }*/
  
  
  /*********************************************/  
  
  
  /**************************************************/

 
  root["name"] = "sezer";
  root["data"]["device_id"]="sezer";
  root["data"]["battery"]=batteryStatus;
  root["data"]["battery2"]=ESP.getVcc();

  String jsonString;
  root.printTo(jsonString);
  char buffer[300];
  root.printTo(buffer, sizeof(buffer));
  if (!mqttClient.publish("data.sezer", jsonString.c_str())) {
    logMqtt("Couldn't sent mqtt message"); 
  }
  StaticJsonBuffer<200> jsonBuffer2;
  JsonObject& root2 = jsonBuffer2.createObject();
  root2["message"] = "data is sent";
  String jsonString2;
  root2.printTo(jsonString2);
  if (!mqttClient.publish("log.sezer",jsonString2.c_str())) {
    logMqtt("Couldn't sent mqtt log"); 
  }
}


void mqttConnect() {
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (mqttClient.connect("isiNemSensoru1", mqttUser, mqttPassword )) {


      
      mqttClient.subscribe("isiNemSensoru1Job");
      mqttClient.setCallback(callback);

      logMqtt("WiFi connected");  
      logMqtt("IP address: ");
      logMqtt(WiFi.localIP().toString());
      
      logMqtt("Connected to mqtt broker");
    } else {
      Serial.print("failed with state ");
      Serial.print(mqttClient.state());
    }
  }
}

void logMqtt(String log) {
  Serial.println(log);
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["message"] = log;
  String jsonString;
  root.printTo(jsonString);
  
  if (!mqttClient.publish("log.sezer", jsonString.c_str())) {
    Serial.println("Couldn't sent mqtt log"); 
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
 
  String msg="";
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    msg += (char)payload[i];
  }

  logMqtt("Message arrived in topic: ");
  logMqtt(topic);
 
  logMqtt("Message:");
  
  Serial.println(msg);
  logMqtt("test");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(msg);
  String job=root["job"];
  logMqtt("job");
  logMqtt(job);

  if (job == "update") {
    String url = root["url"];
    String host = root["host"];
    int port = root["port"];
    String uri = root["uri"];
    logMqtt("url");
    logMqtt(url);
    beginUpdate(url, host, port, uri);
  }
  else if (job == "reset") {
    //resetFunc();
  }
  else if (job == "sleep") {
    int timemicro = root["time"];
    logMqtt("time");
    logMqtt(String(timemicro));
    logMqtt("update3");
    delay(2000);
    ESP.deepSleep(timemicro);
    //WiFi.forceSleepBegin();
    logMqtt("deep sleeped");
    delay(1000);
  }
  else if (job == "config") {
    device_name = root["name"] | device_name.c_str();
    send_when_change = root["send_when_change"] | send_when_change;
    delayMS = root["delayMS"] | delayMS;
    dataSendInterval = root["dataSendInterval"] | dataSendInterval;
    logMqtt("device_name");
    logMqtt(device_name);
    logMqtt("delayMS");
    logMqtt(String(delayMS));
    logMqtt("dataSendInterval");
    logMqtt(String(dataSendInterval));
  }
  
  Serial.println();
  Serial.println("-----------------------");
 
}

void handleIndex() {
  /*if (!is_authentified()) {
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(301);
    return;
  }*/
  
  Serial.println("index");
  String s = MAIN_page;
  s.replace("@@ssid@@", data.ssid);
  s.replace("@@pass@@", data.pass);
  server.send(200, "text/html", s);
  return;
}

void handleSave() {
  logMqtt("saving");
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
  
  data.ssid = server.arg("ssid");
  //logMqtt(newssid);
  data.pass = server.arg("password");
  //logMqtt(newpass);
  
  //server.sendHeader("Location", String("/"), true);
  //server.send(302, "text/plain", "");

  server.send(200, "text/html", "SSID connection information was successfully saved. To access configuration page, please reset your device!");
  
  afterSave();
}

void afterSave() {
  /*
  EEPROM.put(0, data);
  EEPROM.commit();*/


  //EEPROM.get(0, data);
  /*if (wifiSSIDPass != "") {
    int colonIndex = wifiSSIDPass.indexOf(":");
    newssid = wifiSSIDPass.substring(0, colonIndex);
    newpass = wifiSSIDPass.substring(colonIndex);
  }*/
  //saveData();
  Serial.println("data.ssid");
  Serial.println(data.ssid);
  Serial.println("data.pass");
  Serial.println(data.pass);
  
  connectWifi();
  
  server.stop();

  WiFi.mode(WIFI_STA);
}

void connectWifi() {

  if (data.ssid != "" && data.pass != "") {
    WiFi.begin(data.ssid.c_str(), data.pass.c_str());
  
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    mqttConnect();
  }
}

void sensorInfo() {
  sensor_t sensor;
  // Set delay between sensor readings based on sensor details.
  //delayMS = sensor.min_delay / 1000;
  // We start by connecting to a WiFi network
}

void OTAConfig() {
   
   httpUpdater.setup(&server, update_path, update_username, update_password);
}

void beginUpdate(String url, String host, int port, String uri) {
  logMqtt(url + " is using to update");
  ESPhttpUpdate.rebootOnUpdate(true);
  t_httpUpdate_return ret = ESPhttpUpdate.update(url);
  connectWifi();
  switch(ret) {
   case HTTP_UPDATE_FAILED:
       Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
       logMqtt(String(ESPhttpUpdate.getLastError()));
       logMqtt(ESPhttpUpdate.getLastErrorString());
       break;

   case HTTP_UPDATE_NO_UPDATES:
       logMqtt("HTTP_UPDATE_NO_UPDATES");
       break;

   case HTTP_UPDATE_OK:
       logMqtt("HTTP_UPDATE_OK");
       break;

   default:
       logMqtt("Undefined HTTP_UPDATE Code: ");logMqtt(String(ret));

  }
  
  //ESP.restart();
}

void saveData() {
  
  // always use this to "mount" the filesystem
  bool result = SPIFFS.begin();
  Serial.println("SPIFFS opened: " + result);

  // this opens the file "f.txt" in read-mode
  File f = SPIFFS.open("/f.txt", "r");
  
  if (!f) {
    Serial.println("File doesn't exist yet. Creating it");

    // open the file in write mode
    File f = SPIFFS.open("/f.txt", "w");
    if (!f) {
      Serial.println("file creation failed");
    }
    // now write two lines in key/value style with  end-of-line characters
    f.println("ssid=" + data.ssid);
    f.println("password=" + data.pass);
  } else {
    // we could open the file
    while(f.available()) {
      //Lets read line by line from the file
      String line = f.readStringUntil('\n');
      Serial.println(line);
      String a = line.substring(line.indexOf("="));
      Serial.println(a);
    }
  }
  f.close();
}

