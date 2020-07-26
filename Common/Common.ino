#include <ESP8266httpUpdate.h>

#include <Adafruit_Sensor.h>

#include "Arduino.h"

#include <DHT.h>
#include <DHT_U.h>

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

#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <WiFiClientSecure.h>
#include <time.h>

// Required for LIGHT_SLEEP_T delay mode
extern "C" {
#include "user_interface.h"
}

#include "index.h"
#include "FS.h"

int delayMS = 10000;
int dataTime = 0;
int dataSendInterval = 60000 * 10;

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

String device_name = "sezer";
String _firmwareVersion = "8";

extern "C" {
  uint16 readvdd33(void);
}

ADC_MODE(ADC_VCC);

struct {
    String ssid = "Sezer";
    String pass = "RAPtor1234";
    float deepSleepTime = 10;
  } data;

void setup() {
  Serial.begin(115200);
  
  delay(100);

  Serial.println("ADC_VCC integer");
  Serial.println(ADC_VCC);
  
  sensorInfo();
  
  WiFi.mode(WIFI_AP_STA);

  String APname = "Device_" + device_name + "_AP";
  String APpass = "RAPtor1234";
  
  WiFi.softAPConfig(IPAddress(192, 168, 5, 1), IPAddress(192, 168, 5, 0), IPAddress(255, 255, 255, 0));
  WiFi.softAP(APname.c_str(), APpass.c_str());

  IPAddress myIP = WiFi.softAPIP();
  Serial.println("HotSpt IP: " + myIP);

  server.on("/", handleIndex);
  server.on("/save", handleSave);
  //server.on("/login", handleLogin);
  server.begin();
  
  Serial.println("data.ssid: " + data.ssid);
  Serial.println("data.pass: " + data.pass);

  connectWifi();
  server.handleClient();
  
  if (WiFi.status() == WL_CONNECTED) {

     checkFirmware();
     checkConfig();
     post(prepareData());
     Serial.println("Sleeping");
     ESP.deepSleep(data.deepSleepTime * 1000000); // 10s.
  }
}

void loop() {
}

String prepareData() {

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["data"] = jsonBuffer.createObject();
  
  uint16 batteryStatus = readvdd33();
  
  float temperature;
  float humidity;

  
  temperature = -1;
  humidity = -1;
  root["data"]["temp"]=temperature;
  root["data"]["humidity"]=humidity;
  root["data"]["test"] = _firmwareVersion;
  
  root["name"] = "sezer";
  root["data"]["device_id"]="sezer";
  root["data"]["battery"]=batteryStatus;
  root["data"]["battery2"]=ESP.getVcc();
  
  String jsonString;
  root.printTo(jsonString);
  char buffer[300];
  root.printTo(buffer, sizeof(buffer));
  
  return jsonString.c_str();
}

void handleIndex() {
  
  Serial.println("index");
  String s = MAIN_page;
  s.replace("@@ssid@@", data.ssid);
  s.replace("@@pass@@", data.pass);
  server.send(200, "text/html", s);
  return;
}

void handleSave() {
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
  data.pass = server.arg("password");
  server.send(200, "text/html", "SSID connection information was successfully saved. To access configuration page, please reset your device!");
  
  afterSave();
}

void afterSave() {
  
  Serial.println("data.ssid: " + data.ssid);
  Serial.println("data.pass: " + data.pass);
  
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
  }
}

void sensorInfo() {
  sensor_t sensor;
  // Set delay between sensor readings based on sensor details.
  //delayMS = sensor.min_delay / 1000;
  // We start by connecting to a WiFi network
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
const char* fingerprint = "‎94 25 1a 3f f5 d8 91 f7 5f b2 99 79 4e e4 e7 61 e3 1f c4 8c";

void post(String jsonString) {
  Serial.println("Trying to send...host is ");

  Serial.println("Setting time using SNTP");
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr); 
  }
  HTTPClient http;

  http.begin("http://192.168.1.21:8080/api/1.0/telemetry");
  //http.begin(exacturl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");
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

void checkFirmware() {
  Serial.println("Checking firmware");
  
  HTTPClient http;

  http.begin("http://192.168.1.21:8080/api/1.0/firmware?version=" + _firmwareVersion);
  //http.begin(exacturl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");
  

  StaticJsonBuffer<200> postBuffer;
  JsonObject& postRoot = postBuffer.createObject();
  postRoot["version"] = _firmwareVersion;

  String jsonString;
  postRoot.printTo(jsonString);
  char buffer[300];
  postRoot.printTo(buffer, sizeof(buffer));

  http.addHeader("Content-Length", String(jsonString.length()));
  
  int httpCode = http.POST(jsonString.c_str());
  String payload = http.getString();
  StaticJsonBuffer<200> jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(payload);
  String url = root["url"];
  Serial.println(url);
  
  http.end();

  if (url != "" && url != _firmwareVersion) {
    beginUpdate(url);
  }
}

void checkConfig() {
  Serial.println("Checking config");
  
  HTTPClient http;

  http.begin("http://192.168.1.21:8080/api/1.0/config");
  //http.begin(exacturl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");
  
  StaticJsonBuffer<200> postBuffer;
  JsonObject& postRoot = postBuffer.createObject();
  postRoot["version"] = _firmwareVersion;
  postRoot["device_name"] = device_name;

  String jsonString;
  postRoot.printTo(jsonString);
  char buffer[300];
  postRoot.printTo(buffer, sizeof(buffer));

  http.addHeader("Content-Length", String(jsonString.length()));
  
  int httpCode = http.POST(jsonString.c_str());
  String payload = http.getString();
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);
  Serial.println(root["config"]["deepSleepTime"]);
  
  http.end();

  data.deepSleepTime=_config["deepSleepTime"];
}

void beginUpdate(String _url)
{
  ESPhttpUpdate.rebootOnUpdate(true);
  t_httpUpdate_return ret = ESPhttpUpdate.update(_url);
  
  switch(ret) {
   case HTTP_UPDATE_FAILED:
       Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
       Serial.println(String(ESPhttpUpdate.getLastError()));
       Serial.println(ESPhttpUpdate.getLastErrorString());
       break;

   case HTTP_UPDATE_NO_UPDATES:
       Serial.println("HTTP_UPDATE_NO_UPDATES");
       break;

   case HTTP_UPDATE_OK:
       Serial.println("HTTP_UPDATE_OK");
       break;

   default:
      Serial.println("Undefined HTTP_UPDATE Code:");
      Serial.println(String(ret));
  }
  
  //ESP.restart(); 
}

