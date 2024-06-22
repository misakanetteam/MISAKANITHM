#include "report.h"

#include <LittleFS.h>

namespace report {
Adafruit_USBD_HID usb_hid;

Adafruit_NeoPixel *status_led = NULL;

// 是否已启动游戏
bool is_enabled;

uint8_t *data_tx;

uint8_t *data_rx;
}  // namespace report

bool is_enabled() { return report::is_enabled; }

// HID初始化函数
void hid_report_init(Adafruit_NeoPixel *status_led_ptr) {
  // 初始化HID对象
  report::usb_hid.enableOutEndpoint(true);
  report::usb_hid.setBootProtocol(HID_ITF_PROTOCOL_NONE);
  report::usb_hid.setPollInterval(1);

  report::usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  report::usb_hid.setReportCallback(get_report_callback, set_report_callback);

  report::usb_hid.begin();

  // 初始化内存
  report::data_tx = (uint8_t *)malloc(34);
  report::data_rx = (uint8_t *)malloc(62);
  memset(report::data_tx, 0, 34);
  memset(report::data_rx, 0, 62);
  report::status_led = status_led_ptr;
  report::is_enabled = false;
}

// 获取HID报文
uint8_t *hid_report_get() { return report::data_rx; }

// 发送HID报文
void hid_report_send() { report::usb_hid.sendReport(0, report::data_tx, 34); }

// 生成HID报文
void hid_report_air(uint8_t air_value) { report::data_tx[0] = air_value; }
void hid_report_gen(uint8_t touch_value[32]) {
  memcpy(report::data_tx + 2, touch_value, 32);
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t get_report_callback(uint8_t report_id, hid_report_type_t report_type,
                             uint8_t *buffer, uint16_t reqlen) {
  (void)report_id;
  (void)report_type;

  if (reqlen != 34) return 0;

  memset(buffer, 1, 34);

  LittleFS.end();

  report::status_led->clear();
  report::status_led->show();

  Serial.end();

  report::is_enabled = true;

  return reqlen;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
// 当接受到数据时会触发中断, 自动运行这个函数
void set_report_callback(uint8_t report_id, hid_report_type_t report_type,
                         uint8_t const *buffer, uint16_t bufsize) {
  // This example doesn't use multiple report and report ID
  (void)report_id;
  (void)report_type;

  if (bufsize != 62) return;

  memcpy(report::data_rx, buffer, 62);
}