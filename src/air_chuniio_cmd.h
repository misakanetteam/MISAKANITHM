#include <Arduino.h>
#include <Adafruit_VL53L0X.h>
#include <Keyboard.h>

#include "defs.h"

uint8_t KeyCode[6] = {//键值列表
  '/', '.', '\'', ';', ']', '[',
};
void KeyCheck(Adafruit_VL53L0X vl53l0x[VL53L0X_COUNT]) {//按钮和AIR检查
    bool air_key_sensors[6] = {false, false, false, false, false, false};
    // 扫描VL53L0X
    for (int i = 0; i < VL53L0X_COUNT; i++) {
        uint16_t range = vl53l0x[i].readRange();
        if (AIR1_HEIGHT - AIR_RANGE <= range <= AIR1_HEIGHT + AIR_RANGE) {
            air_key_sensors[0] = true;
        }
        if (AIR2_HEIGHT - AIR_RANGE <= range <= AIR2_HEIGHT + AIR_RANGE) {
            air_key_sensors[1] = true;
        }
        if (AIR3_HEIGHT - AIR_RANGE <= range <= AIR3_HEIGHT + AIR_RANGE) {
            air_key_sensors[2] = true;
        }
        if (AIR4_HEIGHT - AIR_RANGE <= range <= AIR4_HEIGHT + AIR_RANGE) {
            air_key_sensors[3] = true;
        }
        if (AIR5_HEIGHT - AIR_RANGE <= range <= AIR5_HEIGHT + AIR_RANGE) {
            air_key_sensors[4] = true;
        }
        if (AIR6_HEIGHT - AIR_RANGE <= range <= AIR6_HEIGHT + AIR_RANGE) {
            air_key_sensors[5] = true;
        }
    }

    for (int i = 0; i < 6; i++) {
        if (air_key_sensors[i]) {
            Keyboard.press(KeyCode[i]);
        }
        else {
            Keyboard.release(KeyCode[i]);
        }
    }
}
