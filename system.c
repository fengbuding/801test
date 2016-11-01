#include "system.h"



void delay(u32 i)
{
    while(i--);
}
void Delay_MicroSeconds(u32 numofus)
{
    u32 i; 
    u8 u=50;
    for(i=0;i<numofus;i++)
    {/*
        __NOP ;        
        __NOP ;       
        __NOP ;
        __NOP ;*/
      delay(u);
    }
}