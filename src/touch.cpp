#include <Arduino.h>
#include <Wire.h>

#include "touch.h"

// 写入触摸芯片配置
// 应先不连接副板启动，L触摸初始化完成后，10s内连接副板
void touch_first_setup()
{
    CY8CMBR3xxx_Configure(0x37, CY8CMBR3116_LQXI_configuration_CapL);
    delay(10000);
    CY8CMBR3xxx_Configure(0x37, CY8CMBR3116_LQXI_configuration_CapR);
}

bool touch_setup()
{
    if (!CY8CMBR3xxx_Configure(0x22, CY8CMBR3116_LQXI_configuration_CapL))
        return false;
    if (!CY8CMBR3xxx_Configure(0x33, CY8CMBR3116_LQXI_configuration_CapR))
        return false;
    return true;
}

uint32 touch_get()
{
    uint16 BufferL;
    uint16 BufferR;
    CY8CMBR3xxx_ReadDualByte(0x22u, CY8CMBR3xxx_BUTTON_STAT, &BufferL);
    CY8CMBR3xxx_ReadDualByte(0x33u, CY8CMBR3xxx_BUTTON_STAT, &BufferR);
    return (uint32)(BufferL << 16 | BufferR);
}

void touch_wake()
{
    Wire.beginTransmission(0x00);
    Wire.write(0x00);
    Wire.endTransmission();
}