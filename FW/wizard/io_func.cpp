/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2026 Per-Simon Saal
 */
 
#include "io_func.h"
#include "my_globals.h"

// Pin Output functions 
uint8_t led_function(uint8_t state){
  if(!state){
    //Serial.println("HIGH");
    digitalWrite(ledPin, HIGH);
    state = 1;
  }
  else if(state){
    //Serial.println("LOW");
    digitalWrite(ledPin, LOW);
    state = 0;    
  }
  return state;
}

void set_ldo_Pin(bool on){
  digitalWrite(shdnldoPin, on);
}

void set_op_Pin(bool on){
  digitalWrite(shdnopPin, on);
}

void set_led_Pin(bool on){
  digitalWrite(ledPin, on);
}

// Pin Input functions
bool get_ready_Pin(){
  return digitalRead(readyPin);
}

bool get_charge_Pin(){
  return digitalRead(chargingPin);
}

bool get_buttonboot_Pin(){
  // TODO: entprellung einbauen, Interrupt?
  return digitalRead(buttonbootPin);
}

bool get_led_Pin(){
  return digitalRead(ledPin);
}

bool get_ldo_Pin(){
  return digitalRead(shdnldoPin);
}

bool get_op_Pin(){
  return digitalRead(shdnopPin);
}
