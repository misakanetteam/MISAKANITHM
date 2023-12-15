#ifndef __DEFS_H
#define __DEFS_H

#define NUM_LEDS 31
#define WS2812_PIN 15
#define BRIGHTNESS 128

#define VL53L0X_COUNT 3
#define VL53L0X_PIN_START 5
#define VL53L0X_ADDR_START 0x30

#define AIR_RANGE 20

enum air_height {
    AIR1_HEIGHT = 100,
    AIR2_HEIGHT = 150,
    AIR3_HEIGHT = 200,
    AIR4_HEIGHT = 250,
    AIR5_HEIGHT = 300,
    AIR6_HEIGHT = 350
};

#endif