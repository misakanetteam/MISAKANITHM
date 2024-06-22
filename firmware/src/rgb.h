#ifndef __RGB_H
#define __RGB_H

#include <Arduino.h>

typedef union {
  uint32_t RGB888;
  struct {
    uint32_t : 3;
    uint32_t RGB_B : 5;
    uint32_t : 2;
    uint32_t RGB_G : 6;
    uint32_t : 3;
    uint32_t RGB_R : 5;
    uint32_t : 8;
  } Work;
} RGB888_struct;
typedef union {
  uint16_t RGB565;
  struct {
    uint16_t RGB_B : 5;
    uint16_t RGB_G : 6;
    uint16_t RGB_R : 5;
  } Work;
} RGB565_struct;

#endif  // __RGB_H