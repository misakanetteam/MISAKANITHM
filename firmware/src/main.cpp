#include <Adafruit_NeoPixel.h>
#include <Adafruit_TinyUSB.h>
#include <Adafruit_VL53L0X.h>
#include <Arduino.h>
#include <LittleFS.h>
#include <Wire.h>

#include "air.h"
#include "defs.h"
#include "report.h"
#include "touch.h"

Adafruit_NeoPixel ws2812(NUM_LEDS, WS2812_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel status_led(1, 16, NEO_GRB + NEO_KHZ800);
Adafruit_VL53L0X vl53l0x[VL53L0X_COUNT];
bool vl53l0x_enable[VL53L0X_COUNT];

CY8CMBR3116 left_touch;
CY8CMBR3116 right_touch;
uint8_t touch_map[] = TOUCH_MAP;

uint8_t air_check_count = 5;

// core0初始化
void setup() {
  rp2040.idleOtherCore();
  // 初始化状态灯
  status_led.begin();
  status_led.setPixelColor(0, 0, 255, 0);  // 状态灯红色 正在初始化
  status_led.show();

  // 初始化USB
  TinyUSBDevice.setID(USB_VID, USB_PID);  // 设定USB设备vid，pid
  TinyUSBDevice.setProductDescriptor("MISAKANITHM");  // 设定USB设备产品名
  TinyUSBDevice.setManufacturerDescriptor(
      "MisakaNet Team");  // 设定USB设备制造商名

  // 初始化HID
  hid_report_init(&status_led);

  // 初始化CDC
  Serial.begin(115200);

#ifdef _DEBUG
  while (!Serial);
#endif

  // 初始化I2C
  Wire.setSDA(I2C0_SDA);
  Wire.setSCL(I2C0_SCL);
  Wire1.setSDA(I2C1_SDA);
  Wire1.setSCL(I2C1_SCL);
  Wire.begin();
  Wire1.begin();

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
  {
    left_touch._config_data[SENSOR_DEBOUNCE] = 1;
    left_touch._config_data[DEVICE_CFG0] = 0b00000000;
    left_touch._config_data[DEVICE_CFG2] = 0b00011000;
    left_touch._config_data[SENSITIVITY0] = 0b00000000;
    left_touch._config_data[SENSITIVITY1] = 0b00000000;
    left_touch._config_data[SENSITIVITY2] = 0b00000000;
    left_touch._config_data[SENSITIVITY3] = 0b00000000;

    left_touch._config_data[BASE_THRESHOLD0] = 200;
    left_touch._config_data[BASE_THRESHOLD1] = 200;
    left_touch._config_data[FINGER_THRESHOLD2] = 150;
    left_touch._config_data[FINGER_THRESHOLD3] = 150;
    left_touch._config_data[FINGER_THRESHOLD4] = 200;
    left_touch._config_data[FINGER_THRESHOLD5] = 200;
    left_touch._config_data[FINGER_THRESHOLD6] = 150;
    left_touch._config_data[FINGER_THRESHOLD7] = 150;
    left_touch._config_data[FINGER_THRESHOLD8] = 150;
    left_touch._config_data[FINGER_THRESHOLD9] = 150;
    left_touch._config_data[FINGER_THRESHOLD10] = 150;
    left_touch._config_data[FINGER_THRESHOLD11] = 150;
    left_touch._config_data[FINGER_THRESHOLD12] = 150;
    left_touch._config_data[FINGER_THRESHOLD13] = 150;
    left_touch._config_data[FINGER_THRESHOLD14] = 150;
    left_touch._config_data[FINGER_THRESHOLD15] = 150;
  }
  left_touch.begin(0x22, &Wire);

  {
    left_touch._config_data[SENSOR_DEBOUNCE] = 1;
    left_touch._config_data[DEVICE_CFG0] = 0b00000000;
    left_touch._config_data[DEVICE_CFG2] = 0b00011000;
    left_touch._config_data[SENSITIVITY0] = 0b00000000;
    left_touch._config_data[SENSITIVITY1] = 0b00000000;
    left_touch._config_data[SENSITIVITY2] = 0b00000000;
    left_touch._config_data[SENSITIVITY3] = 0b00000000;

    right_touch._config_data[BASE_THRESHOLD0] = 150;
    right_touch._config_data[BASE_THRESHOLD1] = 150;
    right_touch._config_data[FINGER_THRESHOLD2] = 150;
    right_touch._config_data[FINGER_THRESHOLD3] = 150;
    right_touch._config_data[FINGER_THRESHOLD4] = 150;
    right_touch._config_data[FINGER_THRESHOLD5] = 150;
    right_touch._config_data[FINGER_THRESHOLD6] = 200;
    right_touch._config_data[FINGER_THRESHOLD7] = 200;
    right_touch._config_data[FINGER_THRESHOLD8] = 150;
    right_touch._config_data[FINGER_THRESHOLD9] = 150;
    right_touch._config_data[FINGER_THRESHOLD10] = 150;
    right_touch._config_data[FINGER_THRESHOLD11] = 150;
    right_touch._config_data[FINGER_THRESHOLD12] = 150;
    right_touch._config_data[FINGER_THRESHOLD13] = 150;
    right_touch._config_data[FINGER_THRESHOLD14] = 200;
    right_touch._config_data[FINGER_THRESHOLD15] = 200;
  }
  right_touch.begin(0x33, &Wire);

  left_touch.writeAllRegister();
  right_touch.writeAllRegister();

  rp2040.resumeOtherCore();

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
  if (!is_enabled()) {
    char *buf = (char *)malloc(16);
    Serial.readBytesUntil('\n', buf, 16);
    if (memcmp(buf, "bootloader", 10) == 0) {
      rp2040.rebootToBootloader();
    }
  }

  // 发送报文
  if (is_enabled() && !is_transfering_rgb()) hid_report_send();
}

// core1循环
void loop1() {
#ifdef _DEBUG
  uint32_t raw_touch_value = left_touch.touched() << 16 | right_touch.touched();

  uint16_t touch_value[32];
  for (uint8_t i = 0; i < 32; i++) {
    touch_value[touch_map[i]] = ((raw_touch_value >> i) & 1);
  }
  for (int i = 0; i < 32; i++) {
    if (touch_value[i]) {
      ws2812.setPixelColor(i, 255, 255, 255);
    } else {
      ws2812.setPixelColor(i, 0);
    }
    Serial.print(touch_value[i]);
    ws2812.show();
  }
  Serial.println();
#endif
  if (is_enabled()) {
    // 获取传感器数据
    uint32_t raw_touch_value =
        left_touch.touched() << 16 | right_touch.touched();

    uint8_t touch_value[32];
    for (uint8_t i = 0; i < 32; i++) {
      touch_value[touch_map[i]] = ((raw_touch_value >> i) & 1) * 128;
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
