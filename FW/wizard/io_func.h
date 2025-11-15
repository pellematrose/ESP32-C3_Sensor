#include <cstdint>

uint8_t led_function(uint8_t);
void set_ldo_Pin(bool);
void set_op_Pin(bool);
void set_led_Pin(bool);
bool get_ready_Pin();
bool get_charge_Pin();
bool get_buttonboot_Pin();
bool get_led_Pin();
bool get_ldo_Pin();
bool get_op_Pin();