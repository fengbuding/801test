#include "i2c_hal.h"

//SCL   PF1
//SDA   PC15

#define  SDA_LOW   GPIO_ResetBits(GPIOC, GPIO_Pin_15)
#define  SDA_OPEN  GPIO_SetBits(GPIOC, GPIO_Pin_15)////{GPIO_InitTypeDef f;f.GPIO_Pin=GPIO_Pin_7;f.GPIO_Mode=GPIO_Mode_Out_OD;GPIO_Init(GPIOB, &f);}//
#define  SDA_READ  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_15)

#define  SCL_LOW   GPIO_ResetBits(GPIOF, GPIO_Pin_1)
#define  SCL_OPEN  GPIO_SetBits(GPIOF, GPIO_Pin_1) // {GPIO_InitTypeDef f;f.GPIO_Pin=GPIO_Pin_6;f.GPIO_Mode=GPIO_Mode_Out_OD;GPIO_Init(GPIOB, &f);} //
#define  SCL_READ  GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_1)



etError I2c_WaitWhileClockStrtch(u8 timeout);

void i2c_Init(void)
{
	GPIO_InitTypeDef f;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOF, ENABLE);	
	f.GPIO_Pin=GPIO_Pin_15;
	f.GPIO_Speed=GPIO_Speed_50MHz;
	f.GPIO_Mode=GPIO_Mode_Out_OD;
	GPIO_Init(GPIOC, &f);
    f.GPIO_Pin=GPIO_Pin_1;
	f.GPIO_Speed=GPIO_Speed_50MHz;
	f.GPIO_Mode=GPIO_Mode_Out_OD;
	GPIO_Init(GPIOF, &f);
    SCL_OPEN;
    SDA_OPEN;
}
void i2c_StartCondition(void)
{
    SDA_OPEN;
    Delay_MicroSeconds(1);
    SCL_OPEN;
    Delay_MicroSeconds(1);
    SDA_LOW; 
    Delay_MicroSeconds(10);
    SCL_LOW;
    Delay_MicroSeconds(10);
}
void i2c_StopCondition(void)
{
    SCL_LOW;
    Delay_MicroSeconds(1);
    SDA_LOW; 
    Delay_MicroSeconds(1);
    SCL_OPEN;
    Delay_MicroSeconds(10); 
    SDA_OPEN;
    Delay_MicroSeconds(10);   
}
etError I2c_writeByte(u8 txByte)
{
    etError error=NO_ERROR;
    u8 mask;
    for(mask=0x80;mask>0;mask>>=1)
    {
        if((mask&txByte) == 0)
        {
             SDA_LOW;       
        }
        else
        {
             SDA_OPEN ;  
        }
        Delay_MicroSeconds(1);  
        SCL_OPEN;
        Delay_MicroSeconds(5);
        SCL_LOW;
        Delay_MicroSeconds(1);
    }
    //SDA拉高等待ACK拉低
    SDA_OPEN;
    // clk #9 for ack
    SCL_OPEN;
    Delay_MicroSeconds(1);
    if(SDA_READ)
    {
        error =  ACK_ERROR;
    }
    SCL_LOW;
    Delay_MicroSeconds(20);
    return error;
}

etError I2c_ReadByte(u8 *rxByte ,etI2cAck ack,u8 timeout)
{
    etError error=NO_ERROR;
    u8 mask;
    *rxByte=0;
    //释放SDA
    SDA_OPEN;
    for(mask=0x80;mask>0;mask>>=1)
    {
        SCL_OPEN;
        Delay_MicroSeconds(1);
        error = I2c_WaitWhileClockStrtch(timeout);
        Delay_MicroSeconds(3);
        if(SDA_READ)
        {
            *rxByte|=mask;
        }
        SCL_LOW;
        Delay_MicroSeconds(1);    
    }
    if(ack == ACK)
    {
         SDA_LOW;   
    }
    else
    {
        SDA_OPEN;
    }
    Delay_MicroSeconds(1);
    SCL_OPEN;
    Delay_MicroSeconds(5);
    SCL_LOW;
    SDA_OPEN;
    Delay_MicroSeconds(20);
    return error;
}

etError I2c_WaitWhileClockStrtch(u8 timeout)
{
    etError error = NO_ERROR;;
    while(SCL_READ == 0)
    {
      if(timeout-- == 0)
      {
            return TIMEOUT_ERROR;
      }
       Delay_MicroSeconds(1000);
    }
    return error; 
}


