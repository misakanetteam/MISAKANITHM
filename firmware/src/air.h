#ifndef __AIR_H
#define __AIR_H

#include <Arduino.h>

#include <Adafruit_VL53L0X.h>

#include "defs.h"

// AIR扫描
uint8_t air_check(Adafruit_VL53L0X vl53l0x[VL53L0X_COUNT], bool vl53l0x_enable[VL53L0X_COUNT]);
#endif // __AIR_H