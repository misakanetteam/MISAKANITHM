#include "air.h"

uint8_t air_check(Adafruit_VL53L0X vl53l0x[VL53L0X_COUNT],
                  bool vl53l0x_enable[VL53L0X_COUNT]) {
  uint8_t air_value;
  // 扫描VL53L0X
  for (int i = 0; i < VL53L0X_COUNT; i++) {
    if (!vl53l0x_enable) continue;
    if (!vl53l0x[i].isRangeComplete()) continue;
    // vl53l0x[i].waitRangeComplete();
    uint16_t range = vl53l0x[i].readRange();

    // 判断是否在对应范围内
    if (AIR1_HEIGHT - AIR_RANGE <= range && range <= AIR1_HEIGHT + AIR_RANGE) {
      air_value = air_value | 1;
    }
    if (AIR2_HEIGHT - AIR_RANGE <= range && range <= AIR2_HEIGHT + AIR_RANGE) {
      air_value = air_value | 2;
    }
    if (AIR3_HEIGHT - AIR_RANGE <= range && range <= AIR3_HEIGHT + AIR_RANGE) {
      air_value = air_value | 4;
    }
    if (AIR4_HEIGHT - AIR_RANGE <= range && range <= AIR4_HEIGHT + AIR_RANGE) {
      air_value = air_value | 8;
    }
    if (AIR5_HEIGHT - AIR_RANGE <= range && range <= AIR5_HEIGHT + AIR_RANGE) {
      air_value = air_value | 16;
    }
    if (AIR6_HEIGHT - AIR_RANGE <= range && range <= AIR6_HEIGHT + AIR_RANGE) {
      air_value = air_value | 32;
    }
  }

  return air_value;
}
