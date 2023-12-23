#include "air.h"

uint8_t air_check(Adafruit_VL53L0X vl53l0x[VL53L0X_COUNT]) {
    uint8_t air_key_sensors[6] = {0, 0, 0, 0, 0, 0};
    // 扫描VL53L0X
    for (int i = 0; i < VL53L0X_COUNT; i++) {
        uint16_t range = vl53l0x[i].readRange();
        if (AIR1_HEIGHT - AIR_RANGE <= range && range <= AIR1_HEIGHT + AIR_RANGE) {
            air_key_sensors[0] = 1;
        }
        if (AIR2_HEIGHT - AIR_RANGE <= range && range <= AIR2_HEIGHT + AIR_RANGE) {
            air_key_sensors[1] = 1;
        }
        if (AIR3_HEIGHT - AIR_RANGE <= range && range <= AIR3_HEIGHT + AIR_RANGE) {
            air_key_sensors[2] = 1;
        }
        if (AIR4_HEIGHT - AIR_RANGE <= range && range <= AIR4_HEIGHT + AIR_RANGE) {
            air_key_sensors[3] = 1;
        }
        if (AIR5_HEIGHT - AIR_RANGE <= range && range <= AIR5_HEIGHT + AIR_RANGE) {
            air_key_sensors[4] = 1;
        }
        if (AIR6_HEIGHT - AIR_RANGE <= range && range <= AIR6_HEIGHT + AIR_RANGE) {
            air_key_sensors[5] = 1;
        }
    }

    uint8_t air_value = 0;
    for (uint8_t i = 0; i < 6; i++) {
      air_value = air_value * 2 + air_key_sensors[i];
    }

    return air_value;
}
