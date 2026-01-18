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

void connect_wifi(){
  uint8_t timeout = 0;

  load(SETTINGS_ADDRESS, &c, sizeof(c));

  // Connecting to extern Wifi network
  WiFi.setHostname("Thermometer");
  WiFi.mode(WIFI_STA);
  //WiFi.begin(SSID_STA, PW_STA);
  WiFi.begin(c.ssid, c.pass);
  Serial.printf("Connecting to %s\n", c.ssid);
  while (WiFi.status() != WL_CONNECTED && timeout < 12) {
    timeout++;
    delay(500);
    Serial.print(".");
    if(timeout==11) open_ap();
  }
  MDNS.begin("Thermometer");
  Serial.println(" connected!");
  MG_INFO(("Connected; IP: %s", WiFi.localIP().toString().c_str()));
  apmode = 1; // connected to network
}

void open_ap(){
  // Establish a AP
  WiFi.softAP(ssidap, passwordap);
  delay(100);
  IPAddress IP = WiFi.softAPIP();
  Serial.printf("Connected; IP: %s", WiFi.localIP().toString().c_str());
  //MG_INFO(("Connected; IP: %s", WiFi.localIP().toString().c_str()));
  server.begin();
  apmode = 0; // open AP
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
        ESP.restart();
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




