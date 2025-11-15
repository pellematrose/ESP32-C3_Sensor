#include "wifi_manager.h"
#include "WiFi.h"
#include "my_globals.h"
#include "my_functions.h"
#include <ESPmDNS.h>

const char *ssidap = "Thermometer";
const char *passwordap = "123456789";

struct Cred {
  char ssid[25];
  char pass[25];
};

Cred c;
// Create WiFiServer object on port 80
WiFiServer  server(80);

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
}

void open_ap(){
  // Establish a AP
  WiFi.softAP(ssidap, passwordap);
  delay(100);
  IPAddress IP = WiFi.softAPIP();
  Serial.printf("Connected; IP: %s", WiFi.localIP().toString().c_str());
  //MG_INFO(("Connected; IP: %s", WiFi.localIP().toString().c_str()));
  server.begin();
}