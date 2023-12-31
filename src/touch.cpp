#include "touch.h"
#include <Arduino.h>

bool TouchFirstSetup(){
    CY8CMBR3xxx_Configure(0x37,CY8CMBR3116_LQXI_configuration_CapL);
    Delay(300);
    CY8CMBR3xxx_Configure(0x37,CY8CMBR3116_LQXI_configuration_CapR);
}
uint32 TouchGet(){
    uint16 BufferL;
    uint16 BufferR;
    CY8CMBR3xxx_ReadDualByte(0x22u,0xaa,BufferL);
    CY8CMBR3xxx_ReadDualByte(0x33u,0xaa,BufferR);
    return ((uint32)BufferL << 16 | BufferR);
}