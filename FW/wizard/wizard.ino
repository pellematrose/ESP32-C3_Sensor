
#include "src/mongoose_glue.h"
#include "my_globals.h"

//#include <ESPAsyncWebServer.h>
//#include <AsyncTCP.h>
#include <EEPROM.h>
#include <stdbool.h>
#include "my_functions.h"
#include "setup_pins.h"
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include "temp_func.h"
#include "wifi_manager.h"
#include <U8g2lib.h>
#include "oled_gui.h"

#define ADC_CHANNEL1 ADC1_CHANNEL_1  // GPIO1
#define ADC_CHANNEL2 ADC1_CHANNEL_2  // GPIO2
#define ADC_CHANNEL3 ADC1_CHANNEL_3  // GPIO3
#define DEFAULT_VREF 1100           // in mV, typischer Wert

// ADC
esp_adc_cal_characteristics_t adc_chars;

// OLED
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 20, /* data=*/ 21);   // ESP32 Thing, HW I2C with pin remapping


// variables
float temp1 = 0;
float temp2 = 0;
float temp1_15min[15];
float temp2_15min[15];
float temp1_15minavg = 0;
float temp2_15minavg = 0;
uint8_t count = 0;

// - timing
uint32_t now = 0;
uint32_t last1min = 0;
uint32_t last15min = 0;
uint32_t delta1min = 1000;
uint32_t delta15min = 15000;

void setup() {
  Serial.begin(115200);
  //while (!Serial) delay(50);
  delay(100);
  //Serial.setTxTimeoutMs(0);  // avoid blocking if serial not present

  // pin definitions
  setup_pins();

  // OLED
  //u8g2.begin();
  //u8g2.enableUTF8Print();
  
  // Legacy ADC konfigurieren
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC_CHANNEL1, ADC_ATTEN_DB_11); // PT100 1: 0 dB = 0,85V, 2.5dB = 1,3V, 6dB = 2V, 11dB = 3,3V
  adc1_config_channel_atten(ADC_CHANNEL2, ADC_ATTEN_DB_11); // PT100 2
  adc1_config_channel_atten(ADC_CHANNEL3, ADC_ATTEN_DB_11); // Battery voltage
  // Kalibrierung initialisieren
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, &adc_chars);

  EEPROM.begin(512); // oder eine andere Größe, je nach Bedarf
  load(CAL_ADDRESS, (void *)voltages_cal, sizeof(voltages_cal));
  load(ZERO_ADDRESS, (void *)zero_cal, sizeof(zero_cal));

  // Set logging function to serial print
  mg_log_set_fn([](char ch, void *) { Serial.print(ch); }, NULL);
  mg_log_set(MG_LL_DEBUG);

  connect_wifi();

  mongoose_init();
  mongoose_set_http_handlers("battery", my_get_battery, my_set_battery);
  mongoose_set_http_handlers("temperature", my_get_temperature, my_set_temperature);
  mongoose_set_http_handlers("wifi", my_get_wifi, my_set_wifi);
  mongoose_set_http_handlers("debug", my_get_debug, my_set_debug);
  mongoose_set_http_handlers("mqtt", my_get_mqtt, my_set_mqtt);
  mongoose_set_http_handlers("calibration", my_get_calibration, my_set_calibration);
  mongoose_set_http_handlers("btnhigh", my_check_btnhigh, my_start_btnhigh);
  mongoose_set_http_handlers("btnlow", my_check_btnlow, my_start_btnlow);
  mongoose_set_http_handlers("btnhigh2", my_check_btnhigh2, my_start_btnhigh2);
  mongoose_set_http_handlers("btnlow2", my_check_btnlow2, my_start_btnlow2);
  mongoose_set_http_handlers("resetcal1", my_check_resetcal1, my_start_resetcal1);
  mongoose_set_http_handlers("resetcal2", my_check_resetcal2, my_start_resetcal2);
  mongoose_set_http_handlers("tempoffset1", my_check_tempoffset1, my_start_tempoffset1);
  mongoose_set_http_handlers("tempoffset2", my_check_tempoffset2, my_start_tempoffset2);

}

void loop() {
  mongoose_poll();

  now = millis();

  // time loop one
  if(now - last1min > delta1min){
    last1min = now;

    temp1 = get_temperature(1) - zero_cal[0];//&adc_chars);
    temp2 = get_temperature(2) - zero_cal[1];
    Serial.printf("Temperature: %.1f°C, %.1f°C\n", temp1, temp2);
    
    if(count == 15) count = 0; // reset counter when at end of array
    temp1_15min[count] = temp1;
    temp2_15min[count] = temp2;
    count++;
    temp1_15minavg /= 15;
    temp2_15minavg /= 15;

    //oled_temp(temp1, temp2);
  }

  if(now - last15min > delta15min){
    last15min = now;
    // TODO send tempn_15minavg over mqtt if enabled
  }

  /*
  if(digitalRead(buttonboot)==0){
    digitalWrite(ledPin,0);
  }else{
    digitalWrite(ledPin, 1);
  }*/
}

extern "C" int lwip_hook_ip6_input(struct pbuf *p, struct netif *inp) __attribute__((weak));
extern "C" int lwip_hook_ip6_input(struct pbuf *p, struct netif *inp) {
  if (ip6_addr_isany_val(inp->ip6_addr[0].u_addr.ip6)) {
    pbuf_free(p);
    return 1;
  }
  return 0;
}
