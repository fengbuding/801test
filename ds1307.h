#ifndef __shizhong_H
#define __shizhong_H
#include "stm32f10x.h"
#include "bsp_usart1.h"
#include "system.h"

#define u8 unsigned char
#define u16 unsigned int
#define u32 unsigned long
#define ft  float
typedef struct{
	 u8 sec;
	 u8 min;
	 u8 hour;
	 u8 wday;
	 u8 day;
	 u8 mon;
	 u8 year;
}Time;
void RTC_init(void);
void ds1307_Write(u8 WriteAddr,u8 Data);
void SetTime(void);
u8 ds1307_Read(u8 ReadAddr);
void GetTime(Time *time);
#endif