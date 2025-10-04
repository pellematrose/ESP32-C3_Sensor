/* Ablauf:
1. Öffne preferences.h uns schau nach WIFI credentials
2. Versuche mit WIFI zu verbinden - bei Erfolg Schritt 6.
3. Ist das nicht möglich öffne einen Access Point
4. User Verbindet sich mit AP und öffnet die Website, um die Credentials einzugeben
5. Diese werden gespeichert und der Controller neu gestartet, um sich mit dem WIFI zu verbinden.
6. Bei Verbindung wird der Loopcode ausgeführt.
*/


#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <PubSubClient.h>

Preferences preferences;
AsyncWebServer server(80);

const char* softAP_SSID = "ESP32-Setup";
const char* softAP_PASS = "12345678";
const char* mqtt_server = "192.168.178.85";

WiFiClient espClient;
PubSubClient client(espClient);

// Pin definitions
const int LED = 8;
const int BTN = 20;

//time out variables
uint32_t actual_time = 0;
uint32_t start_time = 0;
uint32_t time_out = 1000;
uint32_t mqtt_counter = 0;

// Button debounce
uint32_t now;
uint32_t last_trigger = 0;
uint32_t debounce = 200;
uint32_t btn_flag = 0;

bool connectToWiFi() {
    String ssid = preferences.getString("ssid", "");
    String pass = preferences.getString("pass", "");
    
    if (ssid == "" || pass == "") {
        Serial.println("No saved Wi-Fi credentials.");
        return false;
    }

    WiFi.begin(ssid.c_str(), pass.c_str());
    Serial.print("Connecting to WiFi...");

    for (int i = 0; i < 5; i++) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nConnected to Wi-Fi!");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            return true;
        }
        Serial.print(".");
        delay(1000);
    }

    Serial.println("\nFailed to connect.");
    preferences.clear();
    return false;
}

void startSoftAP() {
    WiFi.softAP(softAP_SSID, softAP_PASS);
    Serial.println("Soft AP started.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<!DOCTYPE html>"
                  "<html lang='en'>"
                  "<head>"
                  "<meta charset='UTF-8'>"
                  "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                  "<title>Wi-Fi Configuration</title>"
                  "<style>"
                  "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f4f4f4; }"
                  "form { background-color: white; padding: 20px; border-radius: 5px; "
                  "box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1); max-width: 400px; margin: auto; }"
                  "input[type='text'], input[type='password'] { width: 95%; padding: 10px; margin: 10px 0; "
                  "border: 1px solid #ccc; border-radius: 5px; }"
                  "input[type='submit'] { background-color: #4CAF50; color: white; padding: 10px; border: none; "
                  "border-radius: 5px; cursor: pointer; width: 100%; }"
                  "input[type='submit']:hover { background-color: #45a049; }"
                  "</style>"
                  "</head>"
                  "<body>"
                  "<form action='/save' method='post'>"
                  "<label for='ssid'>SSID:</label>"
                  "<input type='text' name='ssid' id='ssid' required><br>"
                  "<label for='pass'>Password:</label>"
                  "<input type='password' name='pass' id='pass' required><br>"
                  "<input type='submit' value='Save'>"
                  "</form>"
                  "</body>"
                  "</html>";

    request->send(200, "text/html", html);
});

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
      String newSSID, newPASS;

      if (request->hasParam("ssid", true)) {
          newSSID = request->getParam("ssid", true)->value();
      }
      if (request->hasParam("pass", true)) {
          newPASS = request->getParam("pass", true)->value();
      }

      if (!newSSID.isEmpty() && !newPASS.isEmpty()) {
          preferences.putString("ssid", "newSSID");
          preferences.putString("pass", newPASS);
          
          request->send(200, "text/html", "Credentials saved! Restarting...");
          
          delay(2000);  // Give time for message to send
          ESP.restart();
      } else {
          request->send(400, "text/html", "Invalid input. Try again.");
      }
  });

    server.begin();
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32C3Dev2")) {  // needs to be individual!!!
      client.subscribe("color/red");
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
  Serial.println(messageTemp);
}

void IRAM_ATTR btn_isr(){
  btn_flag = 1;
  last_trigger = millis();
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    preferences.begin("wifi", false);

    pinMode(LED, OUTPUT);
    pinMode(BTN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BTN), btn_isr, FALLING);
    //startSoftAP();
    //connectToWiFi();
    if (!connectToWiFi()) {
        startSoftAP();
        Serial.println("Keine Verbindung");
    }
    delay(500);
    server.begin();

    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

/*  if(btn_flag){
    Serial.println("Btn pressed");
    btn_flag = 0;
  }*/

  int btn_reading = digitalRead(BTN);

  if((millis() - last_trigger > debounce) && !btn_reading && btn_flag){
    // send mqtt
    char btn_readingString[8];
    //dtostrf(btn_reading, 1, 2, btn_readingString);
    client.publish("light/status", "toggle");
    Serial.print("Licht: ");
    Serial.println(btn_reading);
    btn_flag = 0;
    }

  if (WiFi.status() == WL_CONNECTED) {
      // Your main program logic here
      if(millis() - start_time > time_out){
        start_time = millis();
        if(digitalRead(LED)){
          digitalWrite(LED, LOW);
        }
        else if(!digitalRead(LED)){
          digitalWrite(LED, HIGH);
        }
      }
  }
  else{
    digitalWrite(LED, LOW);
    delay(100);
    digitalWrite(LED, HIGH);
  }
}
