#ifndef __CONFIG_H
#define __CONFIG_H

#include <Arduino.h>

#include "defs.h"
/* working_mode:
 * 0: chuniio raw hid
 * 1: keyboard
 * 2: keyboard with led
 */
#define CONFIG              \
  uint8_t working_mode;     \
  uint8_t sensitivity;      \
  uint8_t key_value[16];    \
  uint8_t air_key_value[6]; \
  uint8_t color_pressed[3]; \
  uint8_t color_nearby[3];  \
  uint8_t color_gap[3];

struct {
  CONFIG
} config;

struct {
  // 1
  uint8_t protocol_version;
  /* 0        0        0        0        0        0        0            0
   * reserved reserved reserved reserved reserved reserved write_config reboot
   */
  uint8_t flags;
  CONFIG
} command_rx;

struct {
  uint8_t protocol_version = 1;
  uint8_t success;
  uint8_t uniqueID[8];
  uint8_t device_version = DEVICE_VERSION;
  uint8_t firmware_version = FIRMWARE_VERSION;

  CONFIG
} command_tx;

bool command_init();
bool command_read();
bool command_execute();

#endif  // __CONFIG_H