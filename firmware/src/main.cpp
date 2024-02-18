#include <Adafruit_NeoPixel.h>
#include <Adafruit_TinyUSB.h>
#include <Adafruit_VL53L0X.h>
#include <Arduino.h>
#include <ArduinoUniqueID.h>
#include <LittleFS.h>
#include <Wire.h>

#include "air.h"
#include "config.h"
#include "defs.h"
#include "report.h"
#include "touch.h"

Adafruit_NeoPixel ws2812(NUM_LEDS, WS2812_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel status_led(1, 16, NEO_GRB + NEO_KHZ800);
Adafruit_VL53L0X vl53l0x[VL53L0X_COUNT];
bool vl53l0x_enable[VL53L0X_COUNT];

uint8_t air_check_count = 5;

// core0初始化
void setup() {
  // 初始化状态灯
  status_led.begin();
  status_led.setPixelColor(0, 0, 255, 0);  // 状态灯红色 正在初始化
  status_led.show();

  // 获取设备序列号
  for (uint8_t i = 0; i < 8; i++) {
    sprintf((char *)(command_tx.uniqueID + i * 2), "%.2X", UniqueID8[i]);
  }

  // 初始化USB
  TinyUSBDevice.setID(USB_VID, USB_PID);  // 设定USB设备vid，pid
  TinyUSBDevice.setProductDescriptor("MISAKANITHM");  // 设定USB设备产品名
  TinyUSBDevice.setManufacturerDescriptor(
      "MisakaNet Team");  // 设定USB设备制造商名

  // 初始化HID
  hid_report_init(&status_led);

  // 初始化CDC
  Serial.begin(115200);

  // 初始化配置功能
  command_init();

  // 初始化I2C
  Wire.setSDA(I2C0_SDA);
  Wire.setSCL(I2C0_SCL);
  Wire1.setSDA(I2C1_SDA);
  Wire1.setSCL(I2C1_SCL);
  Wire.begin();
  Wire1.begin();
  Wire.setClock(100000);

  // 重置VL53L0X
  for (int i = VL53L0X_PIN_START; i < VL53L0X_COUNT + VL53L0X_PIN_START; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
    delay(2);
    digitalWrite(i, HIGH);
    delay(2);
    digitalWrite(i, LOW);
  }
  // 初始化VL53L0X
  for (int i = 0; i < VL53L0X_COUNT; i++) {
    digitalWrite(i + VL53L0X_PIN_START, HIGH);
    delay(2);

    if (vl53l0x[i].begin(0x29, false, &Wire1,
                         Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_SPEED) &&
        vl53l0x[i].setAddress(VL53L0X_ADDR_START + i) &&
        vl53l0x[i].startRangeContinuous())
      vl53l0x_enable[i] = true;
    else
      vl53l0x_enable[i] = false;
  }

  // 初始化CY8CMBR3116
  // touch_setup();

  // 初始化完成
  status_led.setPixelColor(0, 255, 0, 0);  // 状态灯绿色 等待游戏启动
  status_led.show();
}

// core1初始化
void setup1() {
  // 初始化WS2812
  ws2812.begin();
  ws2812.clear();
  ws2812.show();

  // WS2812自检 所有灯闪亮白色光
  ws2812.fill(0xffffff);
  ws2812.show();
  delay(500);
  ws2812.clear();
  ws2812.show();
}

// core0循环
void loop() {
  // 检测Serial指令
  // 启动游戏后停止检测
  if (!is_enabled() && Serial.available()) {
    if (!command_read() || !command_execute()) {
      command_tx.success = 0;
      Serial.write((const char *)&command_tx, sizeof(command_tx));
      LittleFS.end();
      rp2040.reboot();
    }
  }

  // 发送报文
  if (is_enabled() && !is_transfering_rgb()) hid_report_send();
}

// core1循环
void loop1() {
  if (is_enabled()) {
    // 获取传感器数据
    touch_wake();
    // uint32 raw_touch_value = touch_get();
    uint32 raw_touch_value = 0;
    uint8_t touch_value[32];
    for (uint8_t i = 0; i < 32; i++) {
      touch_value[i] = (raw_touch_value >> 31 - i) & 1 * 128;
    }
    if (air_check_count++ == 5) {
      uint8_t air_value = air_check(vl53l0x, vl53l0x_enable);
      air_check_count = 0;

      // 生成报文
      hid_report_gen(air_value, touch_value);
    } else {
      hid_report_gen(touch_value);
    }
    // 更新LED
    if (!is_transfering_rgb()) {
      uint8_t *data_rx = hid_report_get();
      for (uint8_t i = 0; i < 31; i++) {
        uint8_t *ptr = data_rx + i * 3;
        ws2812.setPixelColor(30 - i, *(ptr + 1), *(ptr + 2), *ptr);
      }

      ws2812.show();
    }
  }
}
