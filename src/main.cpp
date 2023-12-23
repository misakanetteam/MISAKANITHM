#include <Arduino.h>

#include <Adafruit_NeoPixel.h>
#include <Adafruit_TinyUSB.h>
#include <Adafruit_VL53L0X.h>
#include <Wire.h>

#include "defs.h"
#include "report.h"

#define RGB(r,g,b) (r|g<<8|b<<16)

Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, true);

Adafruit_NeoPixel ws2812(NUM_LEDS, WS2812_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel status_led(1, 16, NEO_GRB + NEO_KHZ800);
Adafruit_VL53L0X vl53l0x[VL53L0X_COUNT];

struct inputdata data_tx;
struct usb_output_data_1 data_rx_1;
struct usb_output_data_2 data_rx_2;

uint8_t air_check();
uint16_t get_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen);
void set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize);

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
  ws2812.fill(RGB(255,255,255));
  ws2812.show();
  delay(100);
  ws2812.clear();
  ws2812.show();
  
  ws2812.setBrightness(BRIGHTNESS);

  // 初始化USB
  TinyUSBDevice.setSerialDescriptor("OK"); //设定USB设备序列号
  TinyUSBDevice.setID(0x1973,0x2001); //设定USB设备vid，pid
  TinyUSBDevice.setProductDescriptor("MISAKANITHM"); //设定USB设备产品名
  TinyUSBDevice.setManufacturerDescriptor("MisakaNet Team"); //设定USB设备制造商名
  usb_hid.setPollInterval(1); //设定hid报文间隔为1ms，即最大1000hz回报率
  usb_hid.setReportCallback(get_report_callback, set_report_callback); //当电脑向手台发送数据时会调用set_report_callback进行处理
  usb_hid.begin();

  SerialTinyUSB.begin(115200);

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

    vl53l0x[i].begin(0x29, false, &Wire1, Adafruit_VL53L0X::VL53L0X_SENSE_LONG_RANGE);
    vl53l0x[i].setAddress(VL53L0X_ADDR_START + i);
    vl53l0x[i].startRangeContinuous();
  }

  // TODO: 初始化CY8CMBR3116

  // 初始化完成 等待USB连接
  status_led.fill(0x0000ff);
  status_led.show();
  while (!TinyUSBDevice.mounted()) delay(1);  //如果没插入则等待 wait till plugged
  while (!usb_hid.ready()) delay(1);

  status_led.fill(0xff0000);
  status_led.show();

  rp2040.restartCore1();
  rp2040.resumeOtherCore();
}

void loop() {
  uint8_t air_value = air_check();
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

void setup1() {

}

void loop1() {
  for (uint8_t i = 0; i < 20; i++) {
    rgb color = data_rx_1.TouchArea[i];
    ws2812.setPixelColor(i, RGB(color.G, color.R, color.B));
  }
  for (uint i = 0; i < 11; i++) {
    rgb color = data_rx_2.TouchArea[i];
    ws2812.setPixelColor(i + 20, RGB(color.G, color.R, color.B));
  }
}

uint8_t air_check() {
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

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t get_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
  // not used in this example
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;
  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
// 当接受到数据时会触发中断, 自动运行这个函数
void set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
  // This example doesn't use multiple report and report ID
  (void) report_id;
  (void) report_type;
  status_led.fill(0x00ff00);
  status_led.show();
  //buffer即为从电脑收到的数据，首先判断第1byte的数值，来选择解析为data_rx_1还是data_rx_2 (see definition in report.h)
  if (buffer[0] == 0)
  {
    memcpy(&data_rx_1, buffer, bufsize);
  }
  else
  {
    memcpy(&data_rx_2, buffer, bufsize);
  }
  status_led.fill(0xff0000);
  status_led.show();
}