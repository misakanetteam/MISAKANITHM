#include "config.h"

#include <LittleFS.h>

bool command_init()
{
    if (!LittleFS.begin())
        return false;

    if (!LittleFS.exists("/config"))
    {
        if (!LittleFS.format())
            return false;

        File config_file = LittleFS.open("/config", "w");
        const char default_config[30] = {
            0x00, 0x7F,                                     // chuniio raw hid 工作模式，敏感度 127
            0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, // 默认触摸对应按键，keyboard 工作模式及 keyboard with led 工作模式有效
            0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, // 默认触摸对应按键，keyboard 工作模式及 keyboard with led 工作模式有效
            0x30, 0x31, 0x32, 0x33, 0x34, 0x35,             // 默认天键对应按键，keyboard 工作模式及 keyboard with led 工作模式有效
            0x7F, 0x00, 0x23,                               // 默认触摸区按下颜色，keyboard with led 工作模式有效
            0x00, 0x23, 0x7F                                // 默认触摸区间隔颜色，keyboard with led 工作模式有效
        };
        if (config_file.write(default_config) != sizeof(default_config))
            return false;
        config_file.close();
    }

    return true;
}

// 读取指令
bool command_read()
{
    // 获取指令种类
    char command;
    Serial.readBytes(&command, 1);
    switch (command)
    {
    case 0x0: // 重启
        LittleFS.end();
        rp2040.reboot();
        break;

    case 0x1: // 普通指令
        break;

    case 0xff: // 更新固件
        LittleFS.end();
        rp2040.rebootToBootloader();
        break;
    }

    // 获取完整指令
    if (Serial.readBytes((uint8_t *)&command_rx, sizeof(command_rx)) != sizeof(command_rx))
        return false;

    return true;
}
bool command_execute()
{
    // 获取配置
    File config_file = LittleFS.open("/config", "r");

    if (config_file.readBytes((char *)(&command_tx + 12), 30) != 30)
        return false;
    config_file.close();

    if (command_rx.flags << 6 >> 7)     // 写入配置
    {
        File config_file = LittleFS.open("/config", "w");
        if (config_file.write((const char *) (&command_rx + 2), 30) != 30)
            return false;
        config_file.close();
    }

    command_tx.success = 1;
    Serial.write((const char *) &command_tx, sizeof(command_tx));

    if (command_rx.flags << 7 >> 7)     // 需要重启
    {
        LittleFS.end();
        rp2040.reboot();
    }

    return true;
}