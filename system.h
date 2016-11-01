#ifndef _SYSTEM_H_
#define _SYSTEM_H_
#include "stm32f10x.h"

#define u8 unsigned char
#define u16 unsigned int
#define u32 unsigned long
#define ft  float
typedef enum
{
    ACK=0,
    NACK=1,
}etI2cAck;
typedef enum
{
    NO_ERROR= 0x00,
    ACK_ERROR=0x01,
    CHECKSUM_ERROR=0x02,
    TIMEOUT_ERROR=0x04,
    PARM_ERROR=0x80,
}etError;

void delay(u32 i);
void Delay_MicroSeconds(u32 numofus);

#endif

