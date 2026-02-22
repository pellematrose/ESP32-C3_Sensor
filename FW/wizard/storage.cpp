/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2026 Per-Simon Saal
 */

 /* Nach Magic Wert schauen, sonst default werte schreiben.
    Alle Werte aus dem EEPROM laden.
*/

#include "storage.h"

Check ch;

void check_magic(){

  load(MAGIC_ADDRESS, &ch, sizeof(ch)); //TODO implement magic number check
  load(SETTINGS_ADDRESS, &c, sizeof(c));
  load(MQTT_ADDRESS, &mqttdata, sizeof(mqttdata));

  if(ch.magic_num != CONFIG_MAGIC){
    Serial.println("Init default values.");

    memset(&ch, 0, sizeof(ch));

    ch.magic_num = CONFIG_MAGIC;
    save(MAGIC_ADDRESS, &ch, sizeof(ch));

    memset(&c, 0, sizeof(c));
    strncpy(c.ssid, "mywifi", sizeof(c.ssid));
    strncpy(c.pass, "mypass", sizeof(c.pass));
    save(SETTINGS_ADDRESS, &c, sizeof(c));

    memset(&mqttdata, 0, sizeof(mqttdata));
    mqttdata.en = false;
    strncpy(mqttdata.broker, "mybroker", sizeof(mqttdata.broker));
    strncpy(mqttdata.topic_pressure, "topic1", sizeof(mqttdata.topic_pressure));
    strncpy(mqttdata.topic_batvol, "topic2", sizeof(mqttdata.topic_batvol));
    strncpy(mqttdata.topic_temp1, "topic3", sizeof(mqttdata.topic_temp1));
    strncpy(mqttdata.topic_temp2, "topic4", sizeof(mqttdata.topic_temp2));

    save(MQTT_ADDRESS, &mqttdata, sizeof(mqttdata));
  }
}

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