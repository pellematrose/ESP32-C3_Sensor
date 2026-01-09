/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2026 Per-Simon Saal
 */
 
 
#include "temp_func.h"
#include <Arduino.h>
#include "my_globals.h"
#include "my_functions.h"
#include "io_func.h"

#define NO_OF_SAMPLES 100            // Mittelung über mehrere Samples
//#define ADC_CHANNEL 0 //ADC1_CHANNEL_0  // GPIO0 (A0)
#define OP_OFFSET 28

extern esp_adc_cal_characteristics_t adc_chars;

float interpolate(float x, float x0, float x1, float y0, float y1) {
  return y0 + (y1 - y0) * (x - x0) / (x1 - x0);
}

float calc_temp(uint32_t voltage){
  //voltages_cal[0] = load(SETTINGS_ADDRESS, voltages_cal[0], sizeof(*voltages_cal[0]));
  //voltages_cal[1] = load(SETTINGS_ADDRESS+2, voltages_cal[1], sizeof(*voltages_cal[1]));
  //https://www.aeq-web.com/pt100-arduino-code-sketch/
  // Set to 0°C if voltage is higher than max. known point
  if (voltage > voltages[numPoints - 1]) {
    return 0;
  }

  // Find limits for interpolation
  int i = 0;
  while (voltage > voltages[i + 1] && i < numPoints - 1) {
    i++;
  }

  // Interpolate between limits
  return interpolate(voltage, voltages[i], voltages[i + 1], temperatures[i], temperatures[i + 1]);
}

// This function calculates the linear function between the two measurement points and resolves for the temperature.
float temp_func(uint32_t voltage, uint8_t channel){
  // m = T1-T0 / U1-U0
  float m = (temperatures_cal[1] - temperatures_cal[0]) / (voltages_cal[channel*2+1] - voltages_cal[channel*2]);
  // b = y-mx
  float b = temperatures_cal[1] - m * voltages_cal[1];
  // y = mx+b
  float t = m * voltage + b;
  //Serial.printf("m: %.3f\n", m);
  //Serial.printf("b: %.3f\n", b);
  //Serial.printf("t: %.3f\n", t);
  return t;
}

float get_temperature(uint8_t adc_channel){
  float temp = 0;
  uint32_t volt = 0;
  uint8_t ch = 0;
  volt = get_voltage(adc_channel);
  //temp = calc_temp(volt);
  if(adc_channel == 2) ch = 1;      //IO2 = PT100 Channel 1
  else if(adc_channel == 4) ch = 0; //IO4 = PT100 Channel 2
  temp = temp_func(volt, ch);
  return temp;
}

uint32_t get_voltage(uint8_t adc_channel){
  uint32_t adc_reading = 0;
  adc_reading = get_adc_mean(adc_channel);
  // Umrechnung in mV mit Kalibrierung
  //float voltage = (3300.0f/4096.0f) * adc_reading; 
  uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars);
  //Serial.printf("Voltage: %d\n", voltage-OP_OFFSET);
  return (int)voltage-OP_OFFSET;
}

uint32_t get_adc_mean(uint8_t adc_channel){
  uint32_t adc_reading = 0;
  adc1_channel_t channel;
  
  set_ldo_Pin(true);  // turn on 1.8V
  set_op_Pin(true);   // turn on OP
  delay(10); // let the 1.8V settle

  if(adc_channel == 1) channel = ADC1_CHANNEL_1; // IO1
  else if(adc_channel == 2) channel = ADC1_CHANNEL_2; //IO2
  else if(adc_channel == 3) channel = ADC1_CHANNEL_3; //IO3
  // Mittelung über mehrere Messungen
  for (int i = 0; i < NO_OF_SAMPLES; i++) {
    adc_reading += adc1_get_raw(channel);//analogRead(adc_channel);
  }
  adc_reading /= NO_OF_SAMPLES;
  //Serial.printf("ADC: %d\n", adc_reading);

  set_op_Pin(false);   // turn off OP
  set_ldo_Pin(false);  // turn off 1.8V

  return adc_reading;
}

float moving_average(float dat[], int len){
  float sum = 0;
  for(int i=0; i<len; i++){
    sum += dat[i];
  }
  return sum/len;
}

void fill_array(float){
  
}