#include <Arduino.h>

#include <Adafruit_NeoPixel.h>
#include <Adafruit_TinyUSB.h>
#include <Adafruit_VL53L0X.h>
#include <Wire.h>

#include "defs.h"

#include "air.h"
#include "report.h"

#define RGB(r,g,b) (r|g<<8|b<<16)

Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, true);

Adafruit_NeoPixel ws2812(NUM_LEDS, WS2812_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel status_led(1, 16, NEO_GRB + NEO_KHZ800);
Adafruit_VL53L0X vl53l0x[VL53L0X_COUNT];

// core0初始化
void setup() {
  // 单核进行初始化 暂停core1
  rp2040.idleOtherCore();

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
  TinyUSBDevice.setID(0x1973,0x2001); //设定USB设备vid，pid
  TinyUSBDevice.setProductDescriptor("MISAKANITHM"); //设定USB设备产品名
  TinyUSBDevice.setManufacturerDescriptor("MisakaNet Team"); //设定USB设备制造商名
  usb_hid.setPollInterval(1); //设定hid报文间隔为1ms，即最大1000hz回报率
  usb_hid.setReportCallback(get_report_callback, set_report_callback); //当电脑向手台发送数据时会调用set_report_callback进行处理
  usb_hid.begin();

  SerialTinyUSB.begin(115200);

  // 初始化I2C
  Wire.setSDA(0);
  Wire.setSCL(1);
  Wire1.setSDA(2);
  Wire1.setSCL(3);
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

    vl53l0x[i].begin(0x29, false, &Wire1, Adafruit_VL53L0X::VL53L0X_SENSE_LONG_RANGE);
    vl53l0x[i].setAddress(VL53L0X_ADDR_START + i);
    vl53l0x[i].startRangeContinuous();
  }

  // TODO: 初始化CY8CMBR3116

  // 初始化完成 等待USB连接
  status_led.fill(0x0000ff); // 状态灯蓝色 等待连接
  status_led.show();
  while (!TinyUSBDevice.mounted()) delay(1);  //如果没插入则等待 wait till plugged
  while (!usb_hid.ready()) delay(1);

  status_led.fill(0xff0000); // 状态灯绿色 正常工作
  status_led.show();

  // 初始化完成 启动core1
  rp2040.restartCore1();
  rp2040.resumeOtherCore();
}

// core0循环
// 上报触摸和AIR状态
void loop() {
  uint8_t air_value = air_check(vl53l0x);
  data_tx.AIRValue = air_value; //低6bits为红外传感器数据
  data_tx.Buttons = 0x00; //低3bits为三个功能键
  for (int i = 0; i < 32; i ++)
  {
    data_tx.TouchValue[i] = 3; //32个分区的触摸数值
  }
  data_tx.CardStatue = 0; //读卡器卡片状态，0：无卡，1：aime，2：Felica
  for (int i = 0; i < 10; i ++)
  {
    data_tx.CardID[i] = 0; //卡片ID，aime：10字节数据，Felica：8字节数据 + 2字节留空
  }

  usb_hid.sendReport(0, &data_tx, sizeof(data_tx));
}

// core1初始化
// 未使用
void setup1() {}

// core1循环
// 同步WS2812灯效
void loop1() {
  // HID报文长度限制 分两次设置
  for (uint8_t i = 0; i < 20; i++) {
    rgb color = data_rx_1.TouchArea[i];
    ws2812.setPixelColor(i, RGB(color.G, color.R, color.B));
  }
  for (uint i = 0; i < 11; i++) {
    rgb color = data_rx_2.TouchArea[i];
    ws2812.setPixelColor(i + 20, RGB(color.G, color.R, color.B));
  }
}
