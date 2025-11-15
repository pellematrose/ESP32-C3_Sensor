#include <cstdint>
#include <driver/adc.h>
#include <esp_adc_cal.h>

uint8_t led_function(uint8_t);
float interpolate(float, float, float, float, float);
float calc_temp(uint32_t);
float temp_func(uint32_t, uint8_t);
float get_temperature(uint8_t adc_channel);//esp_adc_cal_characteristics_t* adc_chars);
uint32_t get_voltage(uint8_t adc_channel);
uint32_t get_adc_mean(uint8_t adc_channel);