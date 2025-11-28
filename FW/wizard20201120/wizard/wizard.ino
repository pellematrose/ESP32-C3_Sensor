
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
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include "WiFi.h"
#include "io_func.h"

#define ADC_CHANNEL1 ADC1_CHANNEL_1  // GPIO1
#define ADC_CHANNEL2 ADC1_CHANNEL_2  // GPIO2
#define ADC_CHANNEL3 ADC1_CHANNEL_3  // GPIO3
#define DEFAULT_VREF 1100           // in mV, typischer Wert

// ADC
esp_adc_cal_characteristics_t adc_chars;

// I2C
TwoWire myWire = TwoWire(0);

// OLED
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 20, /* data=*/ 21);   // ESP32 Thing, HW I2C with pin remapping

// BMP388
Adafruit_BMP3XX bmp;

// variables
float temp1 = 0;
float temp2 = 0;
float pressure = 0;
uint32_t vol = 0;
bool chrg = 0;
bool rdy = 0;
float temp1_15min[15];
float temp2_15min[15];
float pressure_15min[15];
float temp1_15minavg = 0;
float temp2_15minavg = 0;
float pressure_15minavg = 0;
uint8_t count = 0;

// - timing
uint32_t now = 0;
uint32_t last1min = 0;
uint32_t last15min = 0;
uint32_t delta1min = 60000;  // TODO
uint32_t delta15min = 900000;//15000;// TODO

void setup() {
  Serial.begin(115200);
  delay(100);

  // pin definitions
  setup_pins();
  
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

  myWire.begin(21, 20, 400000); // SDA, SCL, Freq
  if(!bmp.begin_I2C(0x76, &myWire)){
    Serial.println("BMP Error.");
  }
  // OLED
  u8g2.begin();
  u8g2.enableUTF8Print();

  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

  // Set logging function to serial print
  mg_log_set_fn([](char ch, void *) { Serial.print(ch); }, NULL);
  mg_log_set(MG_LL_DEBUG);

  connect_wifi();
  mqtt_setup();
  
  // fill arrays with real values
  // TODO: put in own function
  if (! bmp.performReading()) {
  Serial.println("Failed to perform reading :(");
  return;
  }
  for(int i=0; i<15; i++){
    temp1_15min[i] = get_temperature(1) - zero_cal[0];
    temp2_15min[i] = get_temperature(2) - zero_cal[1];
    pressure_15min[i] = bmp.pressure / 100;
  }
  temp1_15minavg = moving_average(temp1_15min, 15);
  temp2_15minavg = moving_average(temp2_15min, 15);
  pressure_15minavg = moving_average(pressure_15min, 15);
  vol = get_voltage(3) * 2; // 100k : 100k voltage divider
  chrg = get_charge_Pin();
  rdy = get_ready_Pin();
  oled_data(temp1_15minavg, temp2_15minavg, pressure_15minavg);
  // fill array function end

  //esp_sleep_enable_timer_wakeup(900000000);

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
  mongoose_set_http_handlers("reboot", my_check_reboot, my_start_reboot);

}

void loop() {
  mongoose_poll();

  now = millis();

  // mqtt
  if(mqttdata.en == true){
    mqtt_loop();
  }
  
  // time loop one
  if(now - last1min > delta1min){
    last1min = now;

    temp1 = get_temperature(1) - zero_cal[0];
    temp2 = get_temperature(2) - zero_cal[1];
    
    if (! bmp.performReading()) {
      Serial.println("Failed to perform reading :(");
      return;
    }
    pressure = bmp.pressure / 100;
    
    if(count == 15) count = 0; // reset counter when at end of array
    temp1_15min[count] = temp1;
    temp2_15min[count] = temp2;
    pressure_15min[count] = pressure;
    count++;
    temp1_15minavg = moving_average(temp1_15min, 15);
    temp2_15minavg = moving_average(temp2_15min, 15);
    pressure_15minavg = moving_average(pressure_15min, 15);
    //TODO: read battery voltage and charge status
    vol = get_voltage(3) * 2; // 100k : 100k voltage divider
    chrg = get_charge_Pin();
    rdy = get_ready_Pin();

    Serial.printf("%.1f°C, %.1f°C, %.1fhPa, %dmV\n", temp1_15minavg, temp2_15minavg, pressure_15minavg, vol);
    
    oled_data(temp1_15minavg, temp2_15minavg, pressure_15minavg);
  }

  // time loop two
  if(now - last15min > delta15min){
    last15min = now;
      // mqtt
    if(mqttdata.en == true){
      mqtt_publish();
    }
  }
  
  // display battery voltage at button press
  if(get_buttonboot_Pin()==0){
    oled_auxdata(vol);
    delay(1000);
    oled_data(temp1_15minavg, temp2_15minavg, pressure_15minavg);
  }
  /*
  if(digitalRead(buttonboot)==0){
    
  }else{

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
