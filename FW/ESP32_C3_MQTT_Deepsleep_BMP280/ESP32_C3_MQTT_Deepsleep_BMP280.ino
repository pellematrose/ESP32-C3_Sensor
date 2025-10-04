#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP 900
#define SDA_PIN 6
#define SCL_PIN 7

const char* ssid = "Der Fliegende Teppich";
const char* password = "Fliegen1st$choen";
const char* mqtt_server = "192.168.178.85";

const int LED = 8;

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_BMP280 bmp;

float temperature = 0;
float pressure = 0;
bool once = true;
uint32_t wifi_counter = 0;
uint32_t mqtt_counter = 0;
uint32_t bmp_counter = 0;

//time out variables
uint32_t actual_time = 0;
uint32_t start_time = 0;
uint32_t time_out = 10000;

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    if(wifi_counter == 10){
      ESP.restart();
    }
    else{
      wifi_counter++;
      delay(500);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32C3Dev")) {  // needs to be individual!!!
      client.subscribe("esp32/pressure");
    } else {
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

void callback(char* topic, byte* message, unsigned int length) {
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }

  if (String(topic) == "esp32/pressure") {
    //esp_deep_sleep_start();
    Serial.print("Druck: ");
    Serial.println(messageTemp);
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
  }
}

void setup_bmp() {
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!bmp.begin(0x76)) {
    if(bmp_counter == 10){
      ESP.restart();
    }
    else{
      bmp_counter++;
      delay(500);
    }
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //setup_bmp();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  pinMode(LED, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (!once) {
    temperature = bmp.readTemperature();
    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
    client.publish("esp32/temperature", tempString);

    pressure = bmp.readPressure() / 100;
    char presString[8];
    dtostrf(pressure, 1, 2, presString);
    client.publish("esp32/pressure", presString);

    once = true;
    start_time = millis();
  }

  // time out if no mqtt callback arrives, go to deepsleep
  if(millis() - start_time > time_out){
    //esp_deep_sleep_start();
    //Serial.println("Timeout");
  }
}
