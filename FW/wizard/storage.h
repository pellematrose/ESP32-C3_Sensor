/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2026 Per-Simon Saal
 */
#pragma once
#include <EEPROM.h>
#include "wifi_manager.h"
#include "my_globals.h"

#define CONFIG_MAGIC 0xCAFE

struct Check {
  uint16_t magic_num;
};
extern Check ch;

void check_magic();
void load(int addr, void *buf, size_t len);
void save(int addr, const void *buf, size_t len);