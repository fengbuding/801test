#ifndef _I2C_HAL_
#define _I2C_HAL_
#include "system.h"

void i2c_Init(void);
void i2c_StartCondition(void);
void i2c_StopCondition(void);
etError I2c_writeByte(u8 txByte);
etError I2c_ReadByte(u8 *rxByte ,etI2cAck ack,u8 timeout);
etError I2c_WaitWhileClockStrtch(u8 timeout);
#endif
