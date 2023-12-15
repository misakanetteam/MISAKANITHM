#include <Arduino.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_NeoPixel.h>
#include <Keyboard.h>
#include <Wire.h>

#include "defs.h"
#include "air_chuniio_cmd.h"
#include "slider_serial_cmd.h"

#define rgb(r,g,b) (r|g<<8|b<<16)

Adafruit_NeoPixel ws2812(NUM_LEDS, WS2812_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel status_led(1, 16, NEO_GRB + NEO_KHZ800);
Adafruit_VL53L0X vl53l0x[VL53L0X_COUNT];

void setup() {
  rp2040.idleOtherCore();
  // 初始化状态灯
  status_led.begin();
  status_led.setBrightness(BRIGHTNESS);
  status_led.fill(0x00ff00);
  status_led.show();

  // 初始化WS2812
  ws2812.begin();
  ws2812.setBrightness(16);
  ws2812.clear();
  ws2812.show();

  // WS2812自检
  ws2812.fill(rgb(255,255,255));
  ws2812.show();
  delay(100);
  ws2812.clear();
  ws2812.show();
  
  ws2812.setBrightness(BRIGHTNESS);

  status_led.fill(0x0000ff);
  status_led.show();

  // 初始化USB
  Serial.begin(115200);
  Keyboard.begin();

  // 初始化VL53L0X
  for(int i = VL53L0X_PIN_START; i < VL53L0X_COUNT + VL53L0X_PIN_START; i++) {
    pinMode(i, OUTPUT_12MA);
    // 重置VL53L0X
    digitalWrite(i, LOW);
    delay(10);
    digitalWrite(i, HIGH);
    delay(10);
    digitalWrite(i, LOW);
  }

  Wire.setSDA(0);
  Wire.setSCL(1);
  Wire1.setSDA(2);
  Wire1.setSCL(3);
  Wire.begin();
  Wire1.begin();

  for(int i = 0; i < VL53L0X_COUNT; i++) {
    digitalWrite(i + VL53L0X_PIN_START, HIGH);
    delay(10);

    vl53l0x[i].begin(0x29, true, &Wire1, Adafruit_VL53L0X::VL53L0X_SENSE_LONG_RANGE);
    vl53l0x[i].setAddress(VL53L0X_ADDR_START + i);
    vl53l0x[i].startRangeContinuous();
  }

  // TODO: 初始化CY8CMBR3116

  status_led.fill(0xff0000);
  status_led.show();

  rp2040.restartCore1();
  rp2040.resumeOtherCore();
}

void loop() {
  switch (slider_read()) {
    case SLIDER_CMD_SET_LED:
      slider_res_init();
      slider_set_led(ws2812);
      break;
    case SLIDER_CMD_AUTO_SCAN_START:
      auto_scan = true;
      slider_res_init();
      slider_scan();
      break;
    case SLIDER_CMD_AUTO_SCAN_STOP:
      auto_scan = false;
      slider_res_init();
      break;
    case SLIDER_CMD_RESET:
      slider_reset();
      break;
    case SLIDER_CMD_GET_BOARD_INFO:
      slider_get_board_info();
      break;
    default:
      slider_scan();
  }
  slider_write();
}

void setup1() {

}

void loop1() {
  KeyCheck(vl53l0x);
  delay(100);
}