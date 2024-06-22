#ifndef __DEFS_H
#define __DEFS_H

#undef USB_VID
#undef USB_PID
#define USB_VID 0x2E8A
#define USB_PID 0x2002

#define FIRMWARE_VERSION 1

#define CONFIG_FILE "/config"
#define CONFIG_VERSION 1

#define DEFAULT_CONFIG                                                      \
  {                                                                         \
    0x01, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, \
        0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,   \
        0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f          \
  }

#ifdef OFFICIAL_V1

#define DEVICE_TYPE "official"

#define DEVICE_REV 2

#define NUM_LEDS 31
#define WS2812_PIN 27

#define I2C0_SDA 12
#define I2C0_SCL 13
#define I2C1_SDA 14
#define I2C1_SCL 15

#define VL53L0X_COUNT 7
#define VL53L0X_PIN_START 5
#define VL53L0X_ADDR_START 0x30

#define AIR_RANGE 30

#define TOUCH_MAP                                                            \
  {                                                                          \
    17, 19, 21, 23, 16, 18, 20, 22, 24, 26, 28, 30, 25, 27, 29, 31, 6, 4, 2, \
        0, 8, 10, 15, 13, 11, 9, 7, 5, 3, 1, 14, 12                          \
  }

enum air_height {
  AIR1_HEIGHT = 100,
  AIR2_HEIGHT = 150,
  AIR3_HEIGHT = 200,
  AIR4_HEIGHT = 250,
  AIR5_HEIGHT = 300,
  AIR6_HEIGHT = 350
};
#endif  // OFFICIAL_V1

#ifdef OFFICIAL_V2

#define DEVICE_TYPE "official"

#define DEVICE_REV 3

#define NUM_LEDS 31
#define WS2812_PIN 27

#define I2C0_SDA 12
#define I2C0_SCL 13
#define I2C1_SDA 14
#define I2C1_SCL 15

#define VL53L0X_COUNT 7
#define VL53L0X_PIN_START 5
#define VL53L0X_ADDR_START 0x30

#define AIR_RANGE 30

/*#define TOUCH_MAP                                                            \
  {                                                                          \
    1, 0 , 3, 2, 15, 14, 13, 12, 11, 10, 9, 8,6, 7, 4, 5, 17, 16, 19, 18, 31,\
    30, 29, 28, 27, 26, 25, 24, 22, 23, 20, 21                               \
  }*/
#define TOUCH_MAP                                                              \
{                                                                              \
    30, 31, 28, 29, 16, 17, 18, 19, 20, 21, 22, 23, 25, 24, 27, 26, 14, 15, 12,\
    13, 0, 1, 2, 3, 4, 5, 6, 7, 9, 8, 11, 10                                   \
}

enum air_height {
  AIR1_HEIGHT = 100,
  AIR2_HEIGHT = 150,
  AIR3_HEIGHT = 200,
  AIR4_HEIGHT = 250,
  AIR5_HEIGHT = 300,
  AIR6_HEIGHT = 350
};
#endif  // OFFICIAL_V2

#endif  // __DEFS_H

