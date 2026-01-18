/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2026 Per-Simon Saal
 */
 
#include "setup_pins.h"
#include <Arduino.h>
#include "my_globals.h"

// constants
const int readyPin = 0; // lo when finished with charging
//const int temp1Pin = 1; // ADC1_CH1 just for reference
//const int temp2Pin = 2; // ADC1_CH2 just for reference
//const int ubatPin = 3; // ADC1_CH3 just for reference
const int unused1Pin = 4;
const int unused2Pin = 5;
const int shdnopPin = 6; // enable OP for PT100 1 and 2
const int shdnldoPin = 7; // enable 1.8V LDO
const int ledPin = 8;   // blue user led
const int buttonbootPin = 9;  // boot button / user button
const int chargingPin = 10;   // lo while charging
// const int i2c_scl = 20;
// const int i2c_sda = 21;

uint8_t btn_flag = 0;

// interrupt
void IRAM_ATTR rstbtn_isr(){
  btn_flag = 1;
}

void setup_pins(){
  // pins
  pinMode(readyPin, INPUT);
  pinMode(unused1Pin, OUTPUT);
  pinMode(unused2Pin, OUTPUT);
  pinMode(shdnopPin, OUTPUT);
  pinMode(shdnldoPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonbootPin, INPUT_PULLUP);
  pinMode(chargingPin, INPUT);

  digitalWrite(unused1Pin, 0);
  digitalWrite(unused2Pin, 0);
  digitalWrite(shdnopPin, 0);
  digitalWrite(shdnldoPin, 0);
  digitalWrite(ledPin, 0);
  // interrupt
  attachInterrupt(digitalPinToInterrupt(buttonbootPin), rstbtn_isr, FALLING);
}

