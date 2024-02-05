#ifndef __REPORT_H
#define __REPORT_H

#include <Arduino.h>

#include <Adafruit_TinyUSB.h>

// RawHID might never work with multireports, because of OS problems
// therefore we have to make it a single report with no ID. No other HID device will be supported then.
#undef RAWHID_USAGE_PAGE
#define RAWHID_USAGE_PAGE	0xFFC0 // recommended: 0xFF00 to 0xFFFF

#undef RAWHID_USAGE
#define RAWHID_USAGE		0x0C00 // recommended: 0x0100 to 0xFFFF

uint8_t const desc_hid_report[] =
{
  	/*    RAW HID */
    0x06, lowByte(RAWHID_USAGE_PAGE), highByte(RAWHID_USAGE_PAGE),      /* 30 */
    0x0A, lowByte(RAWHID_USAGE), highByte(RAWHID_USAGE),

    0xA1, 0x01,                  /* Collection 0x01 */
    // RawHID is not multireport compatible.
    // On Linux it might work with some modifications,
    // however you are not happy to use it like that.
    //0x85, HID_REPORTID_RAWHID,			 /* REPORT_ID */
    0x75, 0x08,                  /* report size = 8 bits */
    0x15, 0x00,                  /* logical minimum = 0 */
    0x26, 0xFF, 0x00,            /* logical maximum = 255 */

    0x95, 34,        /* report count TX */
    0x09, 0x01,                  /* usage */
    0x81, 0x02,                  /* Input (array) */

    0x95, 63,        /* report count RX */
    0x09, 0x02,                  /* usage */
    0x91, 0x02,                  /* Output (array) */
    0xC0                         /* end collection */ 
};

struct {
  uint8_t AIRValue;         //红外传感器信号，低6bits对应6个传感器
  uint8_t Buttons;         //控制器3个功能按键，低3bits对应3个按键
  uint8_t TouchValue[32];  //32个触摸数值
} data_tx;

struct rgb {
  uint8_t highByte;
  uint8_t lowByte;
};

struct {
  uint8_t enable;
  struct rgb TouchArea[31];  //触摸区域灯
} data_rx;

#endif