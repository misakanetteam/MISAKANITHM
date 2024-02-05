#include <Arduino.h>

#include <Adafruit_NeoPixel.h>
#include <Adafruit_TinyUSB.h>
#include <Adafruit_VL53L0X.h>
#include <Wire.h>

#include "defs.h"

#include "air.h"
#include "report.h"
#include "touch.h"

#define RGB(r,g,b) (r|g<<8|b<<16)

Adafruit_NeoPixel ws2812(NUM_LEDS, WS2812_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel status_led(1, 16, NEO_GRB + NEO_KHZ800);
Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, true);
Adafruit_VL53L0X vl53l0x[VL53L0X_COUNT];

uint16_t get_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen);
void set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize);

void (*resetFunc)() = 0;

bool isEnabled = false;

// core0初始化
void setup() {
  // 初始化状态灯
  status_led.begin();
  status_led.setBrightness(BRIGHTNESS);
  status_led.fill(0x00ff00);  // 状态灯红色 正在初始化
  status_led.show();

  // 初始化WS2812
  ws2812.begin();
  ws2812.setBrightness(BRIGHTNESS);
  ws2812.clear();
  ws2812.show();

  // WS2812自检 所有灯闪亮白色光
  ws2812.fill(0xffffff);
  ws2812.show();
  delay(100);
  ws2812.clear();
  ws2812.show();

  // 初始化USB
  TinyUSBDevice.setSerialDescriptor("OK"); //设定USB设备序列号
  TinyUSBDevice.setID(0x1946, 0x5000); //设定USB设备vid，pid
  TinyUSBDevice.setProductDescriptor("MISAKANITHM"); //设定USB设备产品名
  TinyUSBDevice.setManufacturerDescriptor("MisakaNet Team"); //设定USB设备制造商名
  usb_hid.setPollInterval(1); //设定hid报文间隔为1ms，即最大1000hz回报率
  usb_hid.setReportCallback(get_report_callback, set_report_callback); //当电脑向手台发送数据时会调用set_report_callback进行处理
  usb_hid.begin();

  SerialTinyUSB.begin(115200);
  //while(!Serial);

  // 初始化I2C
  Wire.setSDA(I2C0_SDA);
  Wire.setSCL(I2C0_SCL);
  Wire1.setSDA(I2C1_SDA);
  Wire1.setSCL(I2C1_SCL);
  Wire.begin();
  Wire1.begin();

  // 重置VL53L0X
  for(int i = VL53L0X_PIN_START; i < VL53L0X_COUNT + VL53L0X_PIN_START; i++) {
    pinMode(i, OUTPUT_12MA);
    digitalWrite(i, LOW);
    delay(10);
    digitalWrite(i, HIGH);
    delay(10);
    digitalWrite(i, LOW);
  }
  // 初始化VL53L0X
  for(int i = 0; i < VL53L0X_COUNT; i++) {
    digitalWrite(i + VL53L0X_PIN_START, HIGH);
    delay(10);

    vl53l0x[i].begin(0x29, true, &Wire1, Adafruit_VL53L0X::VL53L0X_SENSE_LONG_RANGE);
    vl53l0x[i].setAddress(VL53L0X_ADDR_START + i);
    vl53l0x[i].startRangeContinuous();
  }

  // 初始化CY8CMBR3116
  touch_setup();

  // 初始化完成
  status_led.fill(0x0000ff); // 状态灯蓝色 若连接正常不会显示蓝色
  status_led.show();
  while (!TinyUSBDevice.mounted()) delay(1);  //如果没插入则等待 wait till plugged
  while (!usb_hid.ready()) delay(1);

  status_led.fill(0xff0000); // 状态灯绿色 等待游戏启动
  status_led.show();
}

// core0循环
void loop() {
  if (isEnabled) {
    // 获取传感器数据
    uint32 raw_touch_value = touch_get();
    //uint32 raw_touch_value = 0;
    uint8_t touch_value[32];
    for (uint8_t i = 0; i < 32; i ++) {
      touch_value[i] = (raw_touch_value >> 31 - i) % 2 * 128;
      Serial.print(touch_value[i]);
      Serial.print(' ');
    }
    Serial.println();
  
    uint8_t air_value = air_check(vl53l0x);

    // 发送报文
    data_tx.AIRValue = air_value; //低6bits为红外传感器数据
    data_tx.Buttons = 0x00; //低3bits为三个功能键

    memcpy(&data_tx.TouchValue, touch_value, 32); //32个分区的触摸数值

    usb_hid.sendReport(0, &data_tx, 35);
  }

  if (Serial.available()) {
    switch(Serial.read()) {
      case 0x0:                     // RST
        resetFunc();
        break;

    }
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t get_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
  // not used in this example
  (void) report_id;
  (void) report_type;
  (void) buffer;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
// 当接受到数据时会触发中断, 自动运行这个函数
void set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
  // This example doesn't use multiple report and report ID
  (void) report_id;
  (void) report_type;

  memcpy(&data_rx, buffer, bufsize);
  
  if(data_rx.enable == 1) {
    isEnabled = true;
    status_led.clear();
    status_led.show();
    return;
  }

  for (uint8_t i = 0; i < 31; i++) {
    rgb color = data_rx.TouchArea[i];
    ws2812.setPixelColor(i, color.highByte << 8 | color.lowByte);
  }
  ws2812.show();
}
