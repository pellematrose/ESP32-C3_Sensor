#include "oled_gui.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

void oled_hello(){
  u8g2.clearBuffer();					// clear the internal memory 
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  u8g2.drawStr(0,10,"Hello World!");	// write something to the internal memory
  u8g2.sendBuffer();					// transfer internal memory to the display
}

void oled_temp(float t1, float t2){
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso18_tf);

  u8g2.setCursor(0, 30);
  u8g2.print("T1: ");
  u8g2.print(t1);
  u8g2.print("°C");

  u8g2.setCursor(0, 60);
  u8g2.print("T2: ");
  u8g2.print(t2);
  u8g2.print("°C");

  u8g2.sendBuffer();
}

void oled_hoermann(){
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_logisoso18_tf);
  u8g2.setCursor(30,40);
  u8g2.print("GTA5");

  u8g2.drawFrame(12,0,116,64);

  u8g2.sendBuffer();
}