/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2026 Per-Simon Saal
 */
 
#include "wifi_manager.h"

#include "my_globals.h"
#include "my_functions.h"
#include <ESPmDNS.h>

#include <Arduino.h>

const char *ssidap = "Thermometer";
const char *passwordap = "123456789";

struct Cred c;
struct md mqttdata;

uint32_t mqtt_counter = 0;
bool apmode = 0; // 0 = AP, 1 = STA

// Create WiFiServer object on port 80
WiFiServer  server(80);

// mqtt
WiFiClient thermoClient;
PubSubClient client(thermoClient);

void connect_wifi()
{
  uint8_t timeout = 0;

  load(SETTINGS_ADDRESS, &c, sizeof(c));

  Serial.println();
  Serial.println("----- WIFI INIT -----");

  WiFi.setHostname("Thermometer1");
  WiFi.mode(WIFI_STA);
  WiFi.begin(c.ssid, c.pass);
  Serial.printf("Connecting to %s\n", c.ssid);

  while (WiFi.status() != WL_CONNECTED && timeout < 12)
  {
    delay(500);
    Serial.print(".");
    timeout++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("STA connected");
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());

    if (MDNS.begin("thermometer1"))
    {
      Serial.println("mDNS started");
    }
    else
    {
      Serial.println("mDNS failed");
    }
    apmode = 1;   // connected to network
    start_webserver();
  }
  else
  {
    Serial.println("STA failed -> switching to AP");
    open_ap();
  }
}

void open_ap()
{
  Serial.println("----- START AP -----");

  WiFi.disconnect(true);
  delay(200);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssidap, passwordap);
  delay(200);
  IPAddress IP = WiFi.softAPIP();
  Serial.printf("AP IP: %s\n", IP.toString().c_str());
  apmode = 0;
  start_webserver();
  Serial.println("AP ready");
}

void start_webserver()
{
  Serial.println("Starting webserver...");
  server.begin();
  delay(50);
  Serial.printf("Free heap: %u\n", ESP.getFreeHeap());
  Serial.println("Webserver started");
}

void mqtt_callback(char* topic, byte* message, unsigned int length) {
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }

  if (String(topic) == "thermometer/ack" && messageTemp == "1") {
    //esp_deep_sleep_start();
  }
}

void mqtt_reconnect() {
  while (!client.connected()) {
    if (client.connect("Thermometer")) {  // needs to be individual!!!
      client.subscribe("thermometer/ack");
    } 
    else {
      if(mqtt_counter == 10){
        //ESP.restart();
        mqtt_counter = 0; // zeile entfernen, wenn restart geht.
      }
      else{
        mqtt_counter++;
        delay(5000);
      }
    }
  }
}

void mqtt_setup(){
  load(MQTT_ADDRESS, &mqttdata, sizeof(mqttdata));
  if(apmode == 1 && mqttdata.en == 1){  // mqtt enabled on website and connected to wifi    
    client.setServer(mqttdata.broker, 1883);
    client.setCallback(mqtt_callback);
  }
  else{
    return;
  } 
}

void mqtt_loop(){
  if(mqttdata.en == true){
    if (!client.connected()) {
        mqtt_reconnect();
    }
    client.loop();
  }
}

void mqtt_publish(){
  char presString[8];
  char temp1String[7];
  char temp2String[7];
  char volString[5];  // must never be bigger than 4200

  snprintf(presString, sizeof(presString), "%.2f", pressure_15minavg);
  snprintf(temp1String, sizeof(temp1String), "%.2f", temp1_15minavg);
  snprintf(temp2String, sizeof(temp2String), "%.2f", temp2_15minavg);
  snprintf(volString, sizeof(volString), "%d", vol);

  client.publish(mqttdata.topic_pressure, presString);
  client.publish(mqttdata.topic_temp1, temp1String);
  client.publish(mqttdata.topic_temp2, temp2String);
  client.publish(mqttdata.topic_batvol, volString);
}




