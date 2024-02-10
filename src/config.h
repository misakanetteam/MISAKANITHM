#ifndef __CONFIG_H
#define __CONFIG_H

#include <Arduino.h>

#include "defs.h"

struct
{
    uint8_t protocol_version;
    /* 0        0        0        0        0              0           0            0
     * reserved reserved reserved reserved get_infomation read_config write_config reboot
     */
    uint8_t flags;
    uint8_t reserved1[4];

    /* 0: chuniio raw hid
     * 1: keyboard
     * 2: keyboard with led
     */
    uint8_t working_mode;

} command_rx;

struct
{
    uint8_t success;
} command_tx;

void commandRead();

#endif