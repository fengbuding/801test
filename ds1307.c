#include "ds1307.h"
#define uchar unsigned char
//SCL   PE3
//SDA   PE5

#define  SDA_LOW_1   GPIO_ResetBits(GPIOE, GPIO_Pin_5)
#define  SDA_OPEN_1  GPIO_SetBits(GPIOE, GPIO_Pin_5)////{GPIO_InitTypeDef f;f.GPIO_Pin=GPIO_Pin_7;f.GPIO_Mode=GPIO_Mode_Out_OD;GPIO_Init(GPIOB, &f);}//
#define  SDA_READ_1  GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_5)

#define  SCL_LOW_1   GPIO_ResetBits(GPIOE, GPIO_Pin_3)
#define  SCL_OPEN_1  GPIO_SetBits(GPIOE, GPIO_Pin_3) // {GPIO_InitTypeDef f;f.GPIO_Pin=GPIO_Pin_6;f.GPIO_Mode=GPIO_Mode_Out_OD;GPIO_Init(GPIOB, &f);} //
#define  SCL_READ_1  GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3)



etError I2c_WaitWhileClockStrtch_1(u8 timeout);

static void i2c_Init_1(void)
{
	GPIO_InitTypeDef f;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	
	f.GPIO_Pin=GPIO_Pin_3|GPIO_Pin_5;
	f.GPIO_Speed=GPIO_Speed_50MHz;
	f.GPIO_Mode=GPIO_Mode_Out_OD;
	GPIO_Init(GPIOE, &f);
    SCL_OPEN_1;
    SDA_OPEN_1;
}
static void delay_1(u32 i)
{
    while(i--);
}
static void Delay_MicroSeconds_1(u32 numofus)
{
    u32 i; 
    u8 u=50;
    for(i=0;i<numofus;i++)
    {/*
        __NOP ;        
        __NOP ;       
        __NOP ;
        __NOP ;*/
      delay_1(u);
    }
}
static void i2c_StartCondition_1(void)
{
    SDA_OPEN_1;
    Delay_MicroSeconds_1(1);
    SCL_OPEN_1;
    Delay_MicroSeconds_1(1);
    SDA_LOW_1; 
    Delay_MicroSeconds_1(10);
    SCL_LOW_1;
    Delay_MicroSeconds_1(10);
}
static void i2c_StopCondition_1(void)
{
    SCL_LOW_1;
    Delay_MicroSeconds_1(1);
    SDA_LOW_1; 
    Delay_MicroSeconds_1(1);
    SCL_OPEN_1;
    Delay_MicroSeconds_1(10); 
    SDA_OPEN_1;
    Delay_MicroSeconds_1(10);   
}
static etError I2c_writeByte_1(u8 txByte)
{
    etError error=NO_ERROR;
    u8 mask;
    for(mask=0x80;mask>0;mask>>=1)
    {
        if((mask&txByte) == 0)
        {
             SDA_LOW_1;       
        }
        else
        {
             SDA_OPEN_1 ;  
        }
        Delay_MicroSeconds(1);  
        SCL_OPEN_1;
        Delay_MicroSeconds(5);
        SCL_LOW_1;
        Delay_MicroSeconds(1);
    }
    //SDA拉高等待ACK拉低
    SDA_OPEN_1;
    // clk #9 for ack
    SCL_OPEN_1;
    Delay_MicroSeconds_1(1);
    if(SDA_READ_1)
    {
        error =  ACK_ERROR;
    }
    SCL_LOW_1;
    Delay_MicroSeconds_1(20);
    return error;
}

static etError I2c_ReadByte_1(u8 *rxByte ,etI2cAck ack,u8 timeout)
{
    etError error=NO_ERROR;
    u8 mask;
    *rxByte=0;
    //释放SDA
    SDA_OPEN_1;
    for(mask=0x80;mask>0;mask>>=1)
    {
        SCL_OPEN_1;
        Delay_MicroSeconds_1(1);
        error = I2c_WaitWhileClockStrtch_1(timeout);
        Delay_MicroSeconds_1(3);
        if(SDA_READ_1)
        {
            *rxByte|=mask;
        }
        SCL_LOW_1;
        Delay_MicroSeconds_1(1);    
    }
    if(ack == ACK)
    {
         SDA_LOW_1;   
    }
    else
    {
        SDA_OPEN_1;
    }
    Delay_MicroSeconds_1(1);
    SCL_OPEN_1;
    Delay_MicroSeconds_1(5);
    SCL_LOW_1;
    SDA_OPEN_1;
    Delay_MicroSeconds_1(20);
    return error;
}

static etError I2c_WaitWhileClockStrtch_1(u8 timeout)
{
    etError error = NO_ERROR;;
    while(SCL_READ_1 == 0)
    {
      if(timeout-- == 0)
      {
            return TIMEOUT_ERROR;
      }
       Delay_MicroSeconds_1(1000);
    }
    return error; 
}



void RTC_init(void)
{
	i2c_Init_1();
	//ds1307_Write(0x00,0x00);//使能时钟，即把0x00地址的最高位置0，让时钟开始工作
	//注意：这里会出一点问题，秒会被清零	
}

void ds1307_Write(u8 WriteAddr,u8 Data)
{	
    u8 temp;
     //转BCD再写入1307
    temp=(Data/10*16)+(Data%10); //16进制转BCD	
	
    i2c_StartCondition_1();  
    I2c_writeByte_1(0xd0);	            //器件写地址
   
    I2c_writeByte_1(WriteAddr);	    //寄存器地址  
     										  		   
    I2c_writeByte_1(temp);            //发送字节  
  		    	   
    i2c_StopCondition_1();             			//产生停止条件
}
void SetTime(void)
{
  u8 timer[]={00,30,14,0,31,10,16};
  ds1307_Write(0x00,timer[0]);  //写秒
  ds1307_Write(0x01,timer[1]);  //写分钟
  ds1307_Write(0x02,timer[2]);  //写小时
  ds1307_Write(0x04,timer[4]);  //写日
  ds1307_Write(0x05,timer[5]); //写月
  ds1307_Write(0x06,timer[6]); //写年
}

u8 ds1307_Read(u8 ReadAddr)
{				  
		u8 temp;		  
		i2c_StartCondition_1();  
		I2c_writeByte_1(0xd0);	   			//器件写地址
		I2c_writeByte_1(ReadAddr);			//寄存器地址
		i2c_StopCondition_1();										//注意！！！这里必须重启iic总线！！
	
		i2c_StartCondition_1(); 
		I2c_writeByte_1(0xd1);        //进入接收模式   
		I2c_ReadByte_1(&temp ,NACK,1);
        i2c_StopCondition_1();                
		return temp;
}
void GetTime(Time *time)
{
  		time->sec=ds1307_Read(0x00);
		time->sec = (time->sec&0x0f) + (((time->sec>>4)&0x07)*10);
		time->min=ds1307_Read(0x01);
		time->min = (time->min&0x0f) + (((time->min>>4)&0x07)*10);
        time->hour=ds1307_Read(0x02);
		time->hour = (time->hour&0x0f) + ((time->hour>>4)&0x01)*10;
		time->wday=ds1307_Read(0x03);
		time->day=ds1307_Read(0x04);
		time->day = (time->day&0x0f) + ((time->day>>4)&0x03)*10;
		time->mon=ds1307_Read(0x05);
		time->mon = (time->mon&0x0f) + (((time->mon>>4)&0x01)*10);
		time->year=ds1307_Read(0x06);
		time->year = (time->year&0x0f) + ((time->year>>4)*10);
        printf("20%02d/%02d/%02d %02d:%02d:%02d\r\n",time->year,time->mon,time->day,time->hour,time->min,time->sec);
}



