/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2026 Per-Simon Saal
 */
 
#pragma once
#include <Arduino.h>

#define LED_PIN 8  // LED pin
//#define WIFI_SSID "wifi_network_name"  // Change this
//#define WIFI_PASS "wifi_password"      // And this
#define SETTINGS_ADDRESS 0  //{WIFI SSID, 25 | WIFI PASS, 25}
#define CAL_ADDRESS 50      // 16 Bytes
#define ZERO_ADDRESS 66     // 8 Bytes
#define MQTT_ADDRESS 74     // 1+50+4*30 Bytes (1*toggle+1*broker+4*topic)

#define I2C_SCL 20
#define I2C_SDA 21

extern const int readyPin;
//extern const int temp1Pin; // ADC1_CH1 just for reference
//extern const int temp2Pin; // ADC1_CH2 just for reference
//extern const int ubatPin; // ADC1_CH3 just for reference
extern const int unused1Pin;
extern const int unused2Pin;
extern const int shdnopPin;
extern const int shdnldoPin;
extern const int ledPin;
extern const int buttonbootPin;
extern const int chargingPin;
extern float temp1;
extern float temp2;
extern float pressure;
extern float temp1_15minavg;
extern float temp2_15minavg;
extern float pressure_15minavg;
extern uint32_t vol;
extern bool chrg;
extern bool rdy;

const uint32_t voltages[] = {164, 427, 1178};       // used for old interpolation function calc_temp()
const float temperatures[] = {-12.77, 1.28, 39.38}; // used for old interpolation function calc_temp()

extern uint32_t voltages_cal[4];     //Temperaturwert vom lose gemessenen Widerstand
const float temperatures_cal[] = {-12, 38.6};  // 95,3R & 115R 
extern float zero_cal[2];

const int numPoints = sizeof(voltages) / sizeof(voltages[0]);

extern bool apmode; // 0 = AP, 1 = STA
