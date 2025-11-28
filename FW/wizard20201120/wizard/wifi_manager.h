#include <PubSubClient.h>
#include <WiFi.h>

extern PubSubClient client; 
extern WiFiClient thermoClient; 

struct md {
  bool en;
  char broker[50];
  char topic_pressure[30];
  char topic_batvol[30];
  char topic_temp2[30];
  char topic_temp1[30];
};
extern md mqttdata;

void connect_wifi();
void open_ap();

void mqtt_setup();
void mqtt_reconnect();
void mqtt_callback(char* topic, byte* message, unsigned int length);
void mqtt_publish();
void mqtt_loop();