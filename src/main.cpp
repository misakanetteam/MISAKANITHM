#include <Arduino.h>
#include <Adafruit_VL53L0X.h>
#include <Adafruit_TinyUSB.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>

#define NUM_LEDS 16
#define WS2812_PIN 4
#define BRIGHTNESS 128

#define VL53L0X_COUNT 6
#define VL53L0X_PIN_START 5
#define VL53L0X_ADDR_START 0x30

Adafruit_NeoPixel ws2812(NUM_LEDS, WS2812_PIN, NEO_GRB + NEO_KHZ800);

Adafruit_VL53L0X vl53l0x[VL53L0X_COUNT];

void setup() {
  TinyUSB_Device_Init(0);
  Serial.begin(115200);

  Wire.setSDA(0);
  Wire.setSCL(1);
  Wire1.setSDA(2);
  Wire1.setSCL(3);

  // 初始化VL53L0X
  for(int i = 0; i < VL53L0X_COUNT; i++) {
    pinMode(i, OUTPUT);
    // 重置VL53L0X
    digitalWrite(i + VL53L0X_PIN_START, LOW);
    delay(10);
    digitalWrite(i + VL53L0X_PIN_START, HIGH);
    delay(10);
    digitalWrite(i + VL53L0X_PIN_START, LOW);
  }

  for(int i = 0; i < VL53L0X_COUNT; i++) {
    digitalWrite(i + VL53L0X_PIN_START, HIGH);
    delay(10);
  
    vl53l0x[i] = Adafruit_VL53L0X();
    if(!vl53l0x[i].begin(VL53L0X_ADDR_START + i)) {
      Serial.print("[Fatal] Failed to boot VL53L0X:");
      Serial.println(i);
    }
  }

  // TODO: 初始化USB

  while( !TinyUSBDevice.mounted() ) delay(1);

  // 初始化WS2812
  ws2812.begin();
  ws2812.setBrightness(BRIGHTNESS);
  ws2812.clear();

  // TODO: 初始化CY8CMBR3116



}


void loop() {

}
