/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2026 Per-Simon Saal
 */
 
#include "oled_gui.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

void oled_hello(){
  u8g2.clearBuffer();					// clear the internal memory 
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  u8g2.drawStr(0,10,"Hello World!");	// write something to the internal memory
  u8g2.sendBuffer();					// transfer internal memory to the display
}

void oled_data(float t1, float t2, float p){
  u8g2.clearBuffer();
  u8g2.setContrast(10);
  u8g2.setFont(u8g2_font_logisoso16_tf);

  u8g2.setCursor(0, 20);
  u8g2.print("T1: ");
  u8g2.print(t1);
  u8g2.print("°C");

  u8g2.setCursor(0, 40);
  u8g2.print("T2: ");
  u8g2.print(t2);
  u8g2.print("°C");

  u8g2.setCursor(0, 60);
  u8g2.print("P: ");
  u8g2.print(p);
  u8g2.print("hPa");

  u8g2.sendBuffer();
}

void oled_auxdata(uint32_t vol){
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso16_tf);

  u8g2.setCursor(0, 20);
  u8g2.print("Bat: ");
  u8g2.print(vol);
  u8g2.print("mV");

  u8g2.setCursor(0, 40);
  u8g2.print("IP: ");
  //u8g2.print("%s", WiFi.localIP().toString().c_str());
  u8g2.sendBuffer();
}

void oled_off(){
  u8g2.clearBuffer();
  u8g2.sendBuffer();
}