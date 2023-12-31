#include "report.h"
#include "defs.h"

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

  //buffer即为从电脑收到的数据，首先判断第1byte的数值，来选择解析为data_rx_1还是data_rx_2 (see definition in report.h)
  if (buffer[0] == 0)
  {
    memcpy(&data_rx_1, buffer, bufsize);
  }
  else
  {
    memcpy(&data_rx_2, buffer, bufsize);
  }
}