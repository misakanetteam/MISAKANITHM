#ifndef __REPORT_H
#define __REPORT_H

#include <Arduino.h>

#include <Adafruit_NeoPixel.h>
#include <Adafruit_TinyUSB.h>

// RawHID might never work with multireports, because of OS problems
// therefore we have to make it a single report with no ID. No other HID device will be supported then.
#undef RAWHID_USAGE_PAGE
#define RAWHID_USAGE_PAGE 0xFFC0 // recommended: 0xFF00 to 0xFFFF

#undef RAWHID_USAGE
#define RAWHID_USAGE 0x0C00 // recommended: 0x0100 to 0xFFFF

uint8_t const desc_hid_report[] =
    {
        /*    RAW HID */
        0x06, lowByte(RAWHID_USAGE_PAGE), highByte(RAWHID_USAGE_PAGE), /* 30 */
        0x0A, lowByte(RAWHID_USAGE), highByte(RAWHID_USAGE),

        0xA1, 0x01, /* Collection 0x01 */
        // RawHID is not multireport compatible.
        // On Linux it might work with some modifications,
        // however you are not happy to use it like that.
        // 0x85, HID_REPORTID_RAWHID,			 /* REPORT_ID */
        0x75, 0x08,       /* report size = 8 bits */
        0x15, 0x00,       /* logical minimum = 0 */
        0x26, 0xFF, 0x00, /* logical maximum = 255 */

        0x95, 34,   /* report count TX */
        0x09, 0x01, /* usage */
        0x81, 0x00, /* Input (array) */

        0x95, 61,   /* report count RX */
        0x09, 0x02, /* usage */
        0x91, 0x00, /* Output (array) */
        0xC0        /* end collection */
};

bool is_enabled();
bool is_transfering_rgb();

void hid_report_init(Adafruit_NeoPixel *status_led_ptr);
uint8_t *hid_report_get();

void hid_report_send();
void hid_report_gen(uint8_t air_value, uint8_t touch_value[32]);
void hid_report_gen(uint8_t touch_value[32]);

uint16_t get_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen);
void set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize);

#endif // __REPORT_H