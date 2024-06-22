#ifndef __CONFIG_H
#define __CONFIG_H

#include <Arduino.h>
#include <LittleFS.h>

#include "defs.h"
#include "touch.h"

void init_config(CY8CMBR3116 *left, CY8CMBR3116 *right);

void threshold_setting_start();

void set_threshold(uint8_t area_id);

uint8_t read_threshold(uint8_t area_id);

void write_threshold_config();
void threshold_setting_loop();
void threshold_setting_end();

#endif  // __CONFIG_H