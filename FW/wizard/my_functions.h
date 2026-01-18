/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2026 Per-Simon Saal
 */
 
#include <Arduino.h>
#include <stddef.h>
#include <stdbool.h>
#include "src/mongoose_glue.h"

#ifdef __cplusplus
extern "C" {
#endif

void load(int addr, void *buf, size_t len);
void save(int addr, const void *buf, size_t len);
void my_get_battery(struct battery *data);
void my_set_battery(struct battery *data);
void my_get_wifi(struct wifi *data);
void my_set_wifi(struct wifi *data);
void my_get_mqtt(struct mqtt *data);
void my_set_mqtt(struct mqtt *data);
void my_get_temperature(struct temperature *data);
void my_set_temperature(struct temperature *data);
void my_get_calibration(struct calibration *data);
void my_set_calibration(struct calibration *data);
void my_get_settings(struct settings *data);
void my_set_settings(struct settings *data);
void my_get_debug(struct debug *data);
void my_set_debug(struct debug *data);

bool my_check_btnlow(void);
void my_start_btnlow(struct mg_str params);
bool my_check_btnhigh(void);
void my_start_btnhigh(struct mg_str params);
bool my_check_tempoffset1(void);
void my_start_tempoffset1(struct mg_str params);
bool my_check_resetcal1(void);
void my_start_resetcal1(struct mg_str params);

bool my_check_btnlow2(void);
void my_start_btnlow2(struct mg_str params);
bool my_check_btnhigh2(void);
void my_start_btnhigh2(struct mg_str params);
bool my_check_tempoffset2(void);
void my_start_tempoffset2(struct mg_str params);
bool my_check_resetcal2(void);
void my_start_resetcal2(struct mg_str params);

bool my_check_reboot(void);
void my_start_reboot(struct mg_str params);
#ifdef __cplusplus
}
#endif
