#ifndef __SLIDER_SERIAL_CMD_H
#define __SLIDER_SERIAL_CMD_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define rgb(r,g,b) (r|g<<8|b<<16)

enum {
  SLIDER_CMD_AUTO_SCAN = 0x01,
  SLIDER_CMD_SET_LED = 0x02,
  SLIDER_CMD_AUTO_SCAN_START = 0x03,
  SLIDER_CMD_AUTO_SCAN_STOP = 0x04,
  SLIDER_CMD_DIVA_UNK_09 = 0x09,
  SLIDER_CMD_DIVA_UNK_0A = 0x0A,
  SLIDER_CMD_RESET = 0x10,
  SLIDER_CMD_GET_BOARD_INFO = 0xF0
};

typedef union slider_packet {
  struct {
    uint8_t sync;
    uint8_t cmd;
    uint8_t size;
    union {
      struct {
        uint8_t led_unk;
        uint8_t leds[96];//触摸点和分隔的灯光数据，从右到左，BRG
      };
      char version[32];
      uint8_t pressure[32];//从右到左的触摸数据
    };
  };
  uint8_t data[128];
} slider_packet_t;

static slider_packet_t req, res;
static uint8_t auto_scan, req_len = 0, req_r, req_sum = 0, req_esc, slider_tx_pending = 0;

uint8_t slider_read() {
  while (Serial.available()) {
    req_r = Serial.read();
    if (req_r == 0xFF) {
      req_len = 0;
      req.sync = req_r;
      req.cmd = 0;
      req.size = req_r;
      req_sum = req_r;
      req_esc = false;
      continue;
    }
    if (req_r == 0xFD) {
      req_esc = true;
      continue;
    }
    if (req_esc) {
      req_esc = false;
      req_r++;
    }
    req_sum += req_r;
    if (!req.cmd) {
      req.cmd = req_r;
      continue;
    }
    if (req.size == 0xFF) {
      req.size = req_r;
      req_len = 3;
      continue;
    }
    if (req_len >= 128) {
      continue;
    }
    req.data[req_len++] = req_r;
    if (req_len == req.size + 4) {
      if (!req_sum) {
        return req.cmd;
      }
    }
  }
  return 0;
}

void slider_write() {
  static short ptr = 0;
  static uint8_t checksum;
  if (res.cmd) {
    slider_tx_pending = res.size + 4;
    ptr = 0;
    checksum = 0;
  } else {
    return;
  }
  while (slider_tx_pending) {
    uint8_t w;
    if (slider_tx_pending == 1) {
      w = -checksum;
    } else {
      w = res.data[ptr];
    }
    checksum += w;
    if ((ptr != 0 && w == 0xFF) || w == 0xFD) {
      Serial.write(0xFD);
      w--;
    }
    Serial.write(w);
    ptr++;
    slider_tx_pending--;
  }
  res.cmd = 0;
}

static void slider_res_init() {//通用回复
  res.sync = 0xFF;
  res.cmd = req.cmd;
  res.size = 0;
  req.cmd = 0;
}

static void slider_reset() {//重置slider，重新初始化触摸
  res.sync = 0xFF;
  res.cmd = SLIDER_CMD_RESET;
  res.size = 0;
  req.cmd = 0;
  // TODO: 重置触摸
}

static void slider_get_board_info() {//返回版本信息
  res.sync = 0xFF;
  res.cmd = SLIDER_CMD_GET_BOARD_INFO;
  res.size = sizeof(res.version);
  strcpy(res.version, "15330   \xA0""06712\xFF""\x90");
  req.cmd = 0;
}

#define CLAMP(val) (val < 0 ? 0 : (val > 255 ? 255 : val))
static void slider_scan() {//触摸扫描
  if (!auto_scan) {
    return;
  }
  res.sync = 0xFF;
  res.cmd = SLIDER_CMD_AUTO_SCAN;
  res.size = sizeof(res.pressure);
  memset(res.pressure, 0, sizeof(res.pressure));
  int16_t bv, fd, pressure;
  // TODO: 触摸扫描
  /*
  for (int i = 0; i < 8; i++) {
    bv = capL.baselineData(i);
    fd = capL.filteredData(i);
    pressure = bv - fd - Threshold + 20;
    res.pressure[31 - (i * 2)] = CLAMP(pressure);
    //res.pressure[31 - (i * 2 + 1)] = 0;
    bv = capR.baselineData(i);
    fd = capR.filteredData(i);
    pressure = bv - fd - Threshold + 20;
    res.pressure[31 - (16 + i * 2)] = CLAMP(pressure);
    //res.pressure[31 - (16 + i * 2 + 1)] = 0;
  }*/
  for (int i = 0; i < 8; i++) {
    res.pressure[31 - (i * 2)] = CLAMP(0);
    //res.pressure[31 - (i * 2 + 1)] = 0;
    res.pressure[31 - (16 + i * 2)] = CLAMP(0);
    //res.pressure[31 - (16 + i * 2 + 1)] = 0;
  }
}

static void slider_set_led(Adafruit_NeoPixel ws2812) {//串口读取led数据
  uint8_t t1, t2;
  for (int i = 0; i < 16; i++) {//BRG
    t1 = 15 - i;
    t2 = i * 6;
    //leds[t1].r = req.leds[t2 + 1];
    //leds[t1].g = req.leds[t2 + 2];
    //leds[t1].b = req.leds[t2 + 0];
    
    ws2812.setPixelColor(t1, rgb(req.leds[t2 + 1], req.leds[t2 + 2], req.leds[t2 + 0]));
  }
  //FastLED.show();
  ws2812.show();
  req.cmd = 0;
}

#endif