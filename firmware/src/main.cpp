#include <Adafruit_NeoPixel.h>
#include <Adafruit_TinyUSB.h>
#include <Adafruit_VL53L0X.h>
#include <Arduino.h>
#include <ArduinoUniqueID.h>
#include <Wire.h>

#include "air.h"
#include "config.h"
#include "defs.h"
#include "report.h"
#include "rgb.h"
#include "touch.h"

#ifdef I2C_ADDR_CONFIG_MODE

CY8CMBR3116 left_touch;
CY8CMBR3116 right_touch;

void setup() {
  Serial.begin(115200);

  Wire.setSDA(I2C0_SDA);
  Wire.setSCL(I2C0_SCL);
  Wire.begin();
  while (!Serial);

  if (left_touch.begin(0x22, &Wire)) {
    Serial.println("0x22 succeeded");
    if (right_touch.begin(0x33, &Wire)) {
      Serial.println("0x33 succeeded");
    } else {
      Serial.println("0x33 not found");
      if (right_touch.begin(0x37, &Wire)) {
        right_touch.set_i2c_address(0x33);
        Serial.println("0x33 succeeded");
      } else {
        Serial.println("0x33 failed");
      }
    }
  } else {
    Serial.println("0x22 not found");
    if (left_touch.begin(0x37, &Wire)) {
      left_touch.set_i2c_address(0x22);

      Serial.println("0x22 succeeded");
    } else {
      Serial.println("0x22 failed");
    }
  }
}

void loop() {}

#else
Adafruit_NeoPixel ws2812(NUM_LEDS, WS2812_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel status_led(1, 16, NEO_GRB + NEO_KHZ800);
Adafruit_VL53L0X vl53l0x[VL53L0X_COUNT];
bool vl53l0x_enable[VL53L0X_COUNT];

CY8CMBR3116 left_touch;
CY8CMBR3116 right_touch;
uint8_t touch_map[] = TOUCH_MAP;

uint8_t air_check_count = 5;
uint32_t LooptimeCount;

bool _initialized = false;

void console_loop(bool initialized) {
  if (Serial.available()) {
    char *buf = (char *)malloc(16);
    Serial.readBytesUntil('\n', buf, 16);
    if (memcmp(buf, "hb", 2) == 0) {  // heartbeat package
      if (initialized)
        Serial.printf("ok\n");
      else if (is_enabled())
        Serial.printf("en\n");
      else
        Serial.printf("wa\n");

    } else if (!is_enabled()) {
      if (memcmp(buf, "di", 2) == 0) {  // device info
        char *buf = (char *)malloc(16);
        for (uint8_t i = 0; i < UniqueIDsize; i++) {
          sprintf(buf + i * 2, "%02X", UniqueID[i]);
        }

        Serial.printf("%d %d %s %s\n", FIRMWARE_VERSION, DEVICE_REV,
                      DEVICE_TYPE, buf);
      } else if (initialized) {
        if (memcmp(buf, "bl", 2) == 0) {  // bootloader
          rp2040.rebootToBootloader();
        } else if (memcmp(buf, "ts", 2) == 0) {  // start threshold setting
          threshold_setting_start();
        } else if (memcmp(buf, "te", 2) == 0) {  // end threshold setting
          threshold_setting_end();
        }
      }
    }
  }

  threshold_setting_loop();
}

// core0初始化
void setup() {
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

  rp2040.resumeOtherCore();

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

#ifdef _DEBUG
  while (!Serial);
#endif
  // 初始化config
  init_config(&left_touch, &right_touch);

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
    delay(5);
    digitalWrite(i, HIGH);
    delay(5);
    digitalWrite(i, LOW);
  }
  // 初始化VL53L0X
  for (int i = 0; i < VL53L0X_COUNT; i++) {
    digitalWrite(i + VL53L0X_PIN_START, HIGH);
    delay(100);

    if (vl53l0x[i].begin(0x29,
#ifdef _DEBUG
                         true,
#else
                         false,
#endif
                         &Wire1, Adafruit_VL53L0X::VL53L0X_SENSE_LONG_RANGE) &&
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
    left_touch._config_data[DEVICE_CFG2] = 0b00000000;
    left_touch._config_data[REFRESH_CTRL] = 1;
    left_touch._config_data[SENSITIVITY0] = 0b00000000;
    left_touch._config_data[SENSITIVITY1] = 0b00000000;
    left_touch._config_data[SENSITIVITY2] = 0b00000000;
    left_touch._config_data[SENSITIVITY3] = 0b00000000;
    left_touch._config_data[BASE_THRESHOLD0] = 0xff;
    left_touch._config_data[BASE_THRESHOLD1] = 0xff;
    left_touch._config_data[FINGER_THRESHOLD2] = 0xff;
    left_touch._config_data[FINGER_THRESHOLD3] = 0xff;
    left_touch._config_data[FINGER_THRESHOLD4] = 0xff;
    left_touch._config_data[FINGER_THRESHOLD5] = 0xff;
    left_touch._config_data[FINGER_THRESHOLD6] = 0xff;
    left_touch._config_data[FINGER_THRESHOLD7] = 0xff;
    left_touch._config_data[FINGER_THRESHOLD8] = 0xff;
    left_touch._config_data[FINGER_THRESHOLD9] = 0xff;
    left_touch._config_data[FINGER_THRESHOLD10] = 0xff;
    left_touch._config_data[FINGER_THRESHOLD11] = 0xff;
    left_touch._config_data[FINGER_THRESHOLD12] = 0xff;
    left_touch._config_data[FINGER_THRESHOLD13] = 0xff;
    left_touch._config_data[FINGER_THRESHOLD14] = 0xff;
    left_touch._config_data[FINGER_THRESHOLD15] = 0xff;
  }
  left_touch.begin(0x22, &Wire);

  {
    right_touch._config_data[SENSOR_DEBOUNCE] = 1;
    right_touch._config_data[DEVICE_CFG0] = 0b00000000;
    right_touch._config_data[DEVICE_CFG2] = 0b00000000;
    right_touch._config_data[REFRESH_CTRL] = 1;
    right_touch._config_data[SENSITIVITY0] = 0b00000000;
    right_touch._config_data[SENSITIVITY1] = 0b00000000;
    right_touch._config_data[SENSITIVITY2] = 0b00000000;
    right_touch._config_data[SENSITIVITY3] = 0b00000000;
    right_touch._config_data[BASE_THRESHOLD0] = 0xff;
    right_touch._config_data[BASE_THRESHOLD1] = 0xff;
    right_touch._config_data[FINGER_THRESHOLD2] = 0xff;
    right_touch._config_data[FINGER_THRESHOLD3] = 0xff;
    right_touch._config_data[FINGER_THRESHOLD4] = 0xff;
    right_touch._config_data[FINGER_THRESHOLD5] = 0xff;
    right_touch._config_data[FINGER_THRESHOLD6] = 0xff;
    right_touch._config_data[FINGER_THRESHOLD7] = 0xff;
    right_touch._config_data[FINGER_THRESHOLD8] = 0xff;
    right_touch._config_data[FINGER_THRESHOLD9] = 0xff;
    right_touch._config_data[FINGER_THRESHOLD10] = 0xff;
    right_touch._config_data[FINGER_THRESHOLD11] = 0xff;
    right_touch._config_data[FINGER_THRESHOLD12] = 0xff;
    right_touch._config_data[FINGER_THRESHOLD13] = 0xff;
    right_touch._config_data[FINGER_THRESHOLD14] = 0xff;
    right_touch._config_data[FINGER_THRESHOLD15] = 0xff;
  }
  right_touch.begin(0x33, &Wire);

  left_touch.writeAllRegister();
  right_touch.writeAllRegister();

  // 初始化完成
  status_led.setPixelColor(0, 255, 0, 0);  // 状态灯绿色 等待游戏启动
  status_led.show();

  _initialized = true;
}

// core1初始化
// core0初始化完成前负责处理控制台基础功能
void setup1() {
  while (!_initialized) console_loop(false);
}

// core0循环
void loop() {
  // 获取触摸数据
  uint32_t raw_touch_value = left_touch.touched() << 16 | right_touch.touched();

  uint8_t touch_value[32];
  for (uint8_t i = 0; i < 32; i++) {
    touch_value[touch_map[i]] = ((raw_touch_value >> i) & 1) * 128;
  }

  hid_report_gen(touch_value);

  // 发送报文
  if (is_enabled()) hid_report_send();
}

// core1循环
void loop1() {
#ifdef _DEBUG
  /*
  uint32_t raw_touch_value = left_touch.touched() << 16 | right_touch.touched();
  uint16_t touch_value[32];
  for (uint8_t i = 0; i < 32; i++) {
    touch_value[touch_map[i]] = ((raw_touch_value >> i) & 1);
  }*/
  uint16_t touch_value[32];
  for (int i = 0; i < 16; i++) {
    touch_value[touch_map[i]] = right_touch.read_touched_data(i);
  }
  for (int i = 0; i < 16; i++) {
    touch_value[touch_map[i + 16]] = left_touch.read_touched_data(i);
  }
  for (int i = 0; i < 32; i++) {
    if (touch_value[i] > 200) {
      ws2812.setPixelColor(i, 255, 255, 255);
    } else {
      ws2812.setPixelColor(i, 0);
    }
    Serial.print(touch_value[i]);
    Serial.print(' ');
    ws2812.show();
  }
  Serial.println();
#endif

  // 检测Serial指令
  console_loop(true);

  if (is_enabled()) {
    // 获取air数据
    if (air_check_count++ == 5) {
      uint8_t air_value = air_check(vl53l0x, vl53l0x_enable);
      air_check_count = 0;

      // 设置air值
      hid_report_air(air_value);
    }
    // 更新LED
    uint8_t *data_rx = hid_report_get();
    for (uint8_t i = 0; i < 31; i++) {
      RGB565_struct *rgb = (RGB565_struct *)(data_rx + (i << 1));
      ws2812.setPixelColor(30 - i, (uint8_t)rgb->Work.RGB_G,
                           (uint8_t)rgb->Work.RGB_R, (uint8_t)rgb->Work.RGB_B);
    }

    ws2812.show();
  }
}

#endif