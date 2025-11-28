#include "my_functions.h"
#include <EEPROM.h>
#include <Arduino.h>
#include "my_globals.h"
#include "temp_func.h"
#include "io_func.h"

uint32_t voltages_cal[] = {0,0,0,0};  // {Sensor 1 CAL Low, Sensor 1 CAL High, Sensor 2 CAL Low, Sensor 2 CAL High}
float zero_cal[] = {0, 0};  // {Sensor 1 Temp 0°C, Sensor 2 Temp 0°C} -> Saves the offset to 0°C. 
bool encalch1 = 0;
bool encalch2 = 0;
uint32_t vol = 0;

void load(int addr, void *buf, size_t len){
  uint8_t *p = (uint8_t *) buf;
  while (len--) *p++ = EEPROM.read(addr++);
}

void save(int addr, const void *buf, size_t len){
  uint8_t *p = (uint8_t *) buf;
  while (len--){
    EEPROM.write(addr++, *p++);
  } 
  EEPROM.commit();
}

void my_get_battery(struct battery *data) {
  vol = get_voltage(3) * 2; // 100k - 100k voltage divider
  bool chrg = get_charge_Pin();
  bool rdy = get_ready_Pin();

  float per   = ((float)vol - 2400.0f) / 1800.0f * 100.0f; // 0% at 2.4V 100% at 4.2V
  data->percent = (int)per;
  data->voltage = vol; 

  // battery is discharging
  if(chrg && rdy){
    strcpy(data->status, "discharge");
  }
  // battery is charging
  else if(!chrg && rdy){
    strcpy(data->status, "charging");
  }
  // charge ended
  else if(chrg && !rdy){
    strcpy(data->status, "full");
  }
  else{ // TODO: detect PWM signal (chip tries to charge but not battery is connected)
    strcpy(data->status, "no bat");
  }
}
void my_set_battery(struct battery *data) {
  //s_battery = *data; // Sync with your device
}

void my_get_wifi(struct wifi *data) {
  load(SETTINGS_ADDRESS, data, sizeof(*data));
}
void my_set_wifi(struct wifi *data) {
  // EEPROM Speicherung
  save(SETTINGS_ADDRESS, data, sizeof(*data));
}

void my_get_temperature(struct temperature *data) {
  data->temp1 = temp1;
  data->temp2 = temp2;
  data->press = pressure;
}
void my_set_temperature(struct temperature *data) {
  
}

void my_get_calibration(struct calibration *data) {
  // send adc value to gui
  data->ench1 = encalch1; //enable calibration for channel 1
  data->ench2 = encalch2; //enable calibration for channel 2

  if(encalch1){
    data->adc = get_voltage(1);
    data->adcl = voltages_cal[0];
    data->adch = voltages_cal[1];
    data->tempcal1 = zero_cal[0];
  }
  if(encalch2){
    data->adc2 = get_voltage(2);
    data->adcl2 = voltages_cal[2];
    data->adch2 = voltages_cal[3];
    data->tempcal2 = zero_cal[1];
  }
}

void my_set_calibration(struct calibration *data) {
  encalch1 = data->ench1;  //enable calibration for channel 1
  encalch2 = data->ench2;  //enable calibration for channel 2
}

void my_get_debug(struct debug *data) {
  // toggle (output)
  data->led = get_led_Pin();
  data->ldo = get_ldo_Pin();
  data->op = get_op_Pin();
  // text (input)
  data->charge = get_charge_Pin();
  data->ready = get_ready_Pin();
  data->boot = get_buttonboot_Pin();
}
void my_set_debug(struct debug *data) {
  // toggle
  set_led_Pin(data->led);
  set_ldo_Pin(data->ldo);
  set_op_Pin(data->op);
}

void my_get_mqtt(struct mqtt *data) {
  load(MQTT_ADDRESS, data, sizeof(*data));
}
void my_set_mqtt(struct mqtt *data) {
  save(MQTT_ADDRESS, data, sizeof(*data));
}

static uint64_t s_action_timeout_btnlow;  // Time when btnlow ends
bool my_check_btnlow(void) {
  return s_action_timeout_btnlow > mg_now(); // Return true if btnlow is in progress
}
void my_start_btnlow(struct mg_str params) {
  MG_DEBUG(("Passed parameters: [%.*s]", params.len, params.buf));
  s_action_timeout_btnlow = mg_now() + 1000; // Start btnlow, finish after 1 second
  //Serial.println("BTN LOW START");
  voltages_cal[0] = get_voltage(1);
  //Save calibration values in eeprom.
  save(CAL_ADDRESS, (const void *)voltages_cal, 16);
}
static uint64_t s_action_timeout_btnhigh;  // Time when btnhigh ends
bool my_check_btnhigh(void) {
  return s_action_timeout_btnhigh > mg_now(); // Return true if btnhigh is in progress
}
void my_start_btnhigh(struct mg_str params) {
  MG_DEBUG(("Passed parameters: [%.*s]", params.len, params.buf));
  s_action_timeout_btnhigh = mg_now() + 1000; // Start btnhigh, finish after 1 second
  //Serial.println("BTN HIGH START");
  voltages_cal[1] = get_voltage(1);
  //Save calibration values in eeprom.
  save(CAL_ADDRESS, (const void *)voltages_cal, 16);
}
static uint64_t s_action_timeout_tempoffset1;  // Time when tempoffset1 ends
bool my_check_tempoffset1(void) {
  return s_action_timeout_tempoffset1 > mg_now(); // Return true if tempoffset1 is in progress
}
void my_start_tempoffset1(struct mg_str params) {
  MG_DEBUG(("Passed parameters: [%.*s]", params.len, params.buf));
  s_action_timeout_tempoffset1 = mg_now() + 1000; // Start tempoffset1, finish after 1 second
  zero_cal[0] = get_temperature(1);
  save(ZERO_ADDRESS, (const void *)zero_cal, 8);
}
static uint64_t s_action_timeout_resetcal1;  // Time when resetcal ends
bool my_check_resetcal1(void) {
  return s_action_timeout_resetcal1 > mg_now(); // Return true if resetcal is in progress
}
void my_start_resetcal1(struct mg_str params) {
  MG_DEBUG(("Passed parameters: [%.*s]", params.len, params.buf));
  s_action_timeout_resetcal1 = mg_now() + 1000; // Start resetcal, finish after 1 second

  voltages_cal[0] = 0;
  voltages_cal[1] = 0;
  zero_cal[0] = 0;
  save(CAL_ADDRESS, (const void *)voltages_cal, 16);
  save(ZERO_ADDRESS, (const void *)voltages_cal, 8);
}

static uint64_t s_action_timeout_btnhigh2;  // Time when btnhigh ends
bool my_check_btnhigh2(void) {
  return s_action_timeout_btnhigh2 > mg_now(); // Return true if btnhigh is in progress
}
void my_start_btnhigh2(struct mg_str params) {
  MG_DEBUG(("Passed parameters: [%.*s]", params.len, params.buf));
  s_action_timeout_btnhigh2 = mg_now() + 1000; // Start btnhigh, finish after 1 second
  //Serial.println("BTN HIGH START");
  voltages_cal[3] = get_voltage(2);
  //Save calibration values in eeprom.
  save(CAL_ADDRESS, (const void *)voltages_cal, 16);//sizeof((const void *)voltages_cal));
}

static uint64_t s_action_timeout_btnlow2;  // Time when btnlow ends
bool my_check_btnlow2(void) {
  return s_action_timeout_btnlow2 > mg_now(); // Return true if btnlow is in progress
}
void my_start_btnlow2(struct mg_str params) {
  MG_DEBUG(("Passed parameters: [%.*s]", params.len, params.buf));
  s_action_timeout_btnlow2 = mg_now() + 1000; // Start btnlow, finish after 1 second
  //Serial.println("BTN LOW START");
  voltages_cal[2] = get_voltage(2);
  //Save calibration values in eeprom.
  save(CAL_ADDRESS, (const void *)voltages_cal, 16);
}
static uint64_t s_action_timeout_tempoffset2;  // Time when tempoffset2 ends
bool my_check_tempoffset2(void) {
  return s_action_timeout_tempoffset2 > mg_now(); // Return true if tempoffset2 is in progress
}
void my_start_tempoffset2(struct mg_str params) {
  MG_DEBUG(("Passed parameters: [%.*s]", params.len, params.buf));
  s_action_timeout_tempoffset2 = mg_now() + 1000; // Start tempoffset2, finish after 1 second
  zero_cal[1] = get_temperature(2);
  save(ZERO_ADDRESS, (const void *)zero_cal, 8);
}
static uint64_t s_action_timeout_resetcal2;  // Time when resetcal ends
bool my_check_resetcal2(void) {
  return s_action_timeout_resetcal2 > mg_now(); // Return true if resetcal is in progress
}
void my_start_resetcal2(struct mg_str params) {
  MG_DEBUG(("Passed parameters: [%.*s]", params.len, params.buf));
  s_action_timeout_resetcal2 = mg_now() + 1000; // Start resetcal, finish after 1 second

  voltages_cal[2] = 0;
  voltages_cal[3] = 0;
  zero_cal[1] = 0;
  save(CAL_ADDRESS, (const void *)voltages_cal, 16);
  save(ZERO_ADDRESS, (const void *)voltages_cal, 8);
  Serial.println("Save values to flash");
}

static uint64_t s_action_timeout_reboot;  // Time when reboot ends
bool my_check_reboot(void) {
  return s_action_timeout_reboot > mg_now(); // Return true if reboot is in progress
}
void my_start_reboot(struct mg_str params) {
  MG_DEBUG(("Passed parameters: [%.*s]", params.len, params.buf));
  s_action_timeout_reboot = mg_now() + 1000; // Start reboot, finish after 1 second
  Serial.println("Restart...");
  ESP.restart();
}