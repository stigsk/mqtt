#include "Adafruit_AM2320.h"
#include "ArduinoJson.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <NTPClient.h> //Note! not the native library, but rather a customized branch implementing function getFormattedDate() 
#include <WiFiUdp.h>

#define mainTopic "main/topic"

const int sleepTime = 900E6; //60E6 = 60seconds, 900E6 = 15minutes,
float currentTemp = 0.0;
float currentHumid = 0.0;

//wifi
const char* wifiSsid = "<ssid>";
const char* wifiPassword = "<wifipw>";

//mqtt
const char* mqttServer = "<ip>";
const int   mqttPort = 1883; //default mqtt port is 1823
const char* mqttUser = "<username>"; 
const char* mqttPassword = "<password>";
const char* mqttClientName = "<clientname>";
const char* temperatureTopic = mainTopic"/temperature";
const char* humidityTopic = mainTopic"/humidity";
const char* attributesTopic = mainTopic"/attributes";

//ntp
const char *ntpPool = "no.pool.ntp.org";  //pick a local pool
const int ntpOffset =  7.2E3;             //7.2E3 = 2 hours
String currentTime;

//json
const size_t plCapacity = JSON_OBJECT_SIZE(3) + 80;
DynamicJsonDocument payload(plCapacity);

const size_t attCapacity = JSON_OBJECT_SIZE(2) + 50;
DynamicJsonDocument attributes(attCapacity);

//init mqtt
WiFiClient espClient;
PubSubClient client(espClient);

//init ntp
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpPool, ntpOffset);

Adafruit_AM2320 am2320 = Adafruit_AM2320();

void setupWifi() {
  //setup wifi
  Serial.println();Serial.print("Connecting to ");Serial.print(wifiSsid);
  WiFi.begin(wifiSsid, wifiPassword);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  Serial.print("IP address: ");Serial.println(WiFi.localIP());
}

void setupMqtt() {

  client.setServer(mqttServer, mqttPort);
  //client.setCallback(callback);
  //client.setCallback(callback);
  while  (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect(mqttClientName, mqttUser, mqttPassword )) {
      Serial.println("..... connected");
    }else{
      Serial.print("..... failed state: ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  Serial.println("Setup");

  setupWifi();
  setupMqtt();

  timeClient.begin();
  am2320.begin();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  
  currentTemp = am2320.readTemperature();
  currentHumid = am2320.readHumidity();
  
  Serial.print(String(temperatureTopic)); Serial.println(currentTemp);
  Serial.print(String(humidityTopic)); Serial.println(currentHumid);

  client.publish(temperatureTopic, String(currentTemp).c_str(), true);
  client.publish(humidityTopic, String(currentHumid).c_str(), true);

  //update ntp
  timeClient.update();
  delay(1000);
  currentTime = timeClient.getFormattedDate();
  Serial.print("Current time: ");Serial.println(currentTime);

  attributes["clientName"] = mqttClientName;
  attributes["timeLastUpdated"] = currentTime;
  
  char buffer1[512];
  size_t n1 = serializeJson(attributes, buffer1);
  client.publish(attributesTopic, buffer1, n1);
  
  payload["currentTempC"] = currentTemp;
  payload["currentHumidPct"] = currentHumid;
  
  char buffer2[512];
  size_t n2 = serializeJson(payload, buffer2);
  client.publish(mainTopic, buffer2, n2);

  serializeJson(attributes, Serial);
  serializeJson(payload, Serial);
  Serial.println();
  
  client.disconnect();
    
  Serial.println("going to sleep...");

  ESP.deepSleep(sleepTime);
  Serial.println("...and this line is never printed.");
}
