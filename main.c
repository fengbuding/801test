#include "stm32f10x.h"
#include "i2c_hal.h"
#include "bsp_usart1.h"
#include "bsp_TiMbase.h" 
#include "bsp_SysTick.h"
#include "ds1307.h"
#include "string.h"
#include "mmc_sd.h"
#include "ff.h"
volatile u32 time = 0; 
char buf[256];
Time  now;
u8 f_busy=0;
//sd
FIL fnew;													/* file objects */
FATFS fs;													/* Work area (file system object) for logical drives */
FRESULT res; 
UINT br, bw;            					/* File R/W count */
char buffer[256]={0};       		  /* file copy buffer */
char textFileBuffer[] = "Welcome to use Wildfire iso stm32 Development Board today is a good day";
char filename_buf[30]="2016_10_25_01_07_10.txt";
char baseline_filename_buf[30]="";
char adc_filename_buf[30]="";
u32 baseline_data1,baseline_data2;
u16 adc1,adc2,adc3;
typedef struct
{
    u16 CCS801_data_adc;
    u16 CCS801_data_co2;
    u16 CCS801_data_tvoc;
    u8 CCS801_data_err;
    u8 add;
}e_strudata;
e_strudata strudata[8];
void DataInit(void)
{
  u8 i;
  for(i=0;i<8;i++)
  {
    strudata[i].CCS801_data_adc=0;
    strudata[i].CCS801_data_co2=0;
    strudata[i].CCS801_data_tvoc=0;
    strudata[i].CCS801_data_err=NO_ERROR;
    strudata[i].add=i;              //写地址0，1，2.....7
  }
}

void BusyIoInit(void)
{  	
    GPIO_InitTypeDef f;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );
	f.GPIO_Pin=GPIO_Pin_4;
	f.GPIO_Mode=GPIO_Mode_IPU;  ///GPIO_Mode_IN_FLOATING     GPIO_Mode_IPD
	GPIO_Init(GPIOA, &f);
}
u8  WaitBusy(void)
{
  u32 t=0;
  do
  {
    if(f_busy==1)
    {
       f_busy=0;
       return 0;
    }
     bsp_DelayMS(10); 
  }while(t++<50);
  f_busy=0;
  return 1;
}
static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  
  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


void EXTI_PA0_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO,ENABLE);
												
	NVIC_Configuration();
    
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;       
  GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IPU;// GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);


  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource4); 
  EXTI_InitStructure.EXTI_Line = EXTI_Line4;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger =EXTI_Trigger_Falling;//EXTI_Trigger_Rising;// ;//EXTI_Trigger_Falling; EXTI_Trigger_Rising_Falling
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure); 
}
etError DATA_Write(u8 add,u8 Reg,u8 data)   //写寄存器
{	
	etError error = NO_ERROR;
	i2c_StartCondition();
    error= I2c_writeByte(add<<1);
    error|= I2c_writeByte(Reg);
    error|= I2c_writeByte(data);
    i2c_StopCondition(); 
	return error;
}

etError DATA_Read(u8 add,u8 Reg,u8 *p)   //读寄存器数据
{
    etError error = NO_ERROR;
    i2c_StartCondition();
    error= I2c_writeByte(add<<1);
    error|= I2c_writeByte(Reg);

    i2c_StartCondition();
    error|= I2c_writeByte((add<<1)|0x01);
    error|= I2c_ReadByte(p,NACK,2);
    i2c_StopCondition(); 
    return error;  
}
void Read_Ccs801(void)
{
        etError error = NO_ERROR;
        u8 i=0;
        u8 data1,data2,data3,data4;
        for(i=0;i<8;i++)
        {
            error|= DATA_Read(strudata[i].add,0,&data1);
            error|= DATA_Read(strudata[i].add,1,&data2);//co2
            error|= DATA_Read(strudata[i].add,2,&data3);
            error|= DATA_Read(strudata[i].add,3,&data4);//tvoc

            strudata[i].CCS801_data_err=error;
            strudata[i].CCS801_data_co2=(data1<<8)|data2;
            strudata[i].CCS801_data_tvoc=(data3<<8)|data4;
            error = NO_ERROR;   //清除err
        }       
}
void Read_baseline(void)
{
    u8 data1,data2,data3,data4;
    DATA_Read(strudata[1].add,4,&data1);
    DATA_Read(strudata[1].add,5,&data2);//
    DATA_Read(strudata[2].add,4,&data3);
    DATA_Read(strudata[2].add,5,&data4);//
    baseline_data1=(data1<<8)|data2;
    baseline_data2=(data3<<8)|data4;
    printf("baselin1:%u baselin2:%u\r\n",baseline_data1,baseline_data2);  
 
    if(now.sec==0)
    {
        res = f_open(&fnew,baseline_filename_buf,FA_OPEN_ALWAYS|FA_WRITE );		 
        if ( res == FR_OK )
        {
            res = f_lseek(&fnew,fnew.fsize);
            sprintf(buffer,"20%02d/%02d/%02d %02d:%02d\r\n",now.year,now.mon,now.day,now.hour,now.min);
            res = f_write(&fnew, buffer, strlen(buffer), &bw);    
            res = f_lseek(&fnew,fnew.fsize);//fnew.fsize   
            sprintf(buffer,"baseline1 %u baseline2 %u\r\n",baseline_data1,baseline_data2);
            res = f_write(&fnew, buffer, strlen(buffer), &bw);  
            res = f_write(&fnew, "\r\n", 2, &bw);
            f_close(&fnew);
        }
    }
}
void Read_adc(void)
{
    u8 data1,data2;
    u8 i;
    for(i=0;i<8;i++)
    {
        DATA_Read(strudata[i].add,6,&data1);
        DATA_Read(strudata[i].add,7,&data2);//
        strudata[i].CCS801_data_adc=(data1<<8)|data2;
    }
    //volt1 = (adc1 * 3300) / 1024;
    for(i=0;i<8;i++)
    {
       if(strudata[i].CCS801_data_err!=NO_ERROR)
        {
          printf("adc 0x%02x --\r\n",strudata[i].add);
        }
        else
        {
           printf("adc 0x%02x %d\r\n",strudata[i].add,strudata[i].CCS801_data_adc);
        }      
        usart2Printf(buf);
        WaitBusy();       
    }   
}
void ShowAdd(void)
{
    u8 i;
    sprintf(buf,"CLS(1);CLS(1);DS48(10,10,'addr',2);DS48(110,10,'CO2',2);DS48(210,10,'TVOC',2);DS48(310,10,'VOL',2);\r\n");    
    usart2Printf(buf);     
    WaitBusy();
    
    for(i=0;i<8;i++)
    {
        sprintf(buf,"DS32(10,%d,'0x%02x',2);\r\n",64+i*32,strudata[i].add);    
        usart2Printf(buf);     
        WaitBusy();
    }
}
void ShowData(void)
{
   u8 i;
   for(i=0;i<8;i++)
   {
        if(strudata[i].CCS801_data_err!=NO_ERROR)
        {
             sprintf(buf,"LABL(32,110,%d,210,'fail',2,1);LABL(32,210,%d,310,'fail',2,1);LABL(32,310,%d,410,'fail',2,1);\r\n",64+i*32,64+i*32,64+i*32);
        }
        else
        {
             sprintf(buf,"LABL(32,110,%d,210,'%d',2,1);LABL(32,210,%d,310,'%d',2,1);LABL(32,310,%d,410,'%d',2,1);\r\n",64+i*32,strudata[i].CCS801_data_co2,64+i*32,strudata[i].CCS801_data_tvoc,64+i*32,(strudata[i].CCS801_data_adc*3300)/1024);
        }      
        usart2Printf(buf);
        WaitBusy();
   }  
}
void ShowRtc()
{
    sprintf(buf,"DS16(400,24,'20%02d/%02d/%02d',2);DS16(400,48,'%02d:%02d:%02d',2);\r\n",now.year,now.mon,now.day,now.hour,now.min,now.sec);
    usart2Printf(buf);
    WaitBusy();
}
void UsartPrint(void)
{
   u8 i;
   for(i=0;i<8;i++)
   {
     printf("801_0x%02x co2=%d tvoc=%d err=%d\r\n",strudata[i].add,strudata[i].CCS801_data_co2,strudata[i].CCS801_data_tvoc,strudata[i].CCS801_data_err);
   }
}
void SD_Write(void)
{
        u8 i;
        if((now.hour==0)&&(now.min==0)&&(now.sec==0))
        {
          sprintf(filename_buf,"20%02d_%02d_%02d.txt",now.year,now.mon,now.day); 
        }
        res = f_open(&fnew,filename_buf,FA_OPEN_ALWAYS|FA_WRITE );		 
        if ( res == FR_OK )
        {
            res = f_lseek(&fnew,fnew.fsize);
            sprintf(buffer,"20%02d/%02d/%02d %02d:%02d:%02d\r\n",now.year,now.mon,now.day,now.hour,now.min,now.sec);
            res = f_write(&fnew, buffer, strlen(buffer), &bw);           
            for(i=0;i<8;i++)
            {
                res = f_lseek(&fnew,fnew.fsize);//fnew.fsize   
                if(strudata[i].CCS801_data_err!=NO_ERROR)
                {
                  sprintf(buffer,"ccs801 0x%02x -- -- --\r\n",strudata[i].add);                  
                }
                else
                {
                   sprintf(buffer,"ccs801 0x%02x %u %u %u\r\n",strudata[i].add,strudata[i].CCS801_data_co2,strudata[i].CCS801_data_tvoc,strudata[i].CCS801_data_adc);
                }
                res = f_write(&fnew, buffer, strlen(buffer), &bw);  
                printf("sd %s\r\n",buffer);
            }
            res = f_write(&fnew, "\r\n", 2, &bw);
            f_close(&fnew);     
           
        }
        else
        {
            printf("sd error open\r\n");
        }
}
void LED_Gpioinit(void)
{
	GPIO_InitTypeDef f;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );
	f.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_5;
	f.GPIO_Speed=GPIO_Speed_50MHz;
	f.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &f);
	GPIO_SetBits(GPIOB, GPIO_Pin_0);	
	GPIO_ResetBits(GPIOB, GPIO_Pin_5); 
}
//master
int main(void)
{
    SysTick_Init();
    bsp_DelayMS(500); //等待从机，串口屏初始化完成
    DataInit();
    UsartInit();
    LED_Gpioinit();
    i2c_Init();
    RTC_init();
    GetTime(&now); 
    
    f_mount(0,&fs);
    sprintf(filename_buf,"20%02d_%02d_%02d_%02d_%02d_%02d.txt",now.year,now.mon,now.day,now.hour,now.min,now.sec);   
    res = f_open(&fnew,filename_buf,FA_OPEN_ALWAYS|FA_WRITE );		 
    if ( res == FR_OK )
    {
       printf("sd create success..");
    }
    else
    {
       printf("%d sd create fail..",res);
    }
    f_close(&fnew); 
    sprintf(baseline_filename_buf,"20%02d_%02d_%02d_%02d_%02d_baseline.txt",now.year,now.mon,now.day,now.hour,now.min);  
    res = f_open(&fnew,baseline_filename_buf,FA_OPEN_ALWAYS|FA_WRITE );   
    f_close(&fnew); 
    EXTI_PA0_Config(); 
    ShowAdd();
    for(;;)
    {	//360ms
        GetTime(&now);   //RTC
        ShowRtc();
        Read_Ccs801(); 
        Read_adc();
        Read_baseline();
        ShowData();
        UsartPrint();        
        SD_Write();
        
        bsp_DelayMS(500); 
        GPIO_WriteBit(GPIOB, GPIO_Pin_0, (BitAction) (1 - GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_0)));
        
    }
}
/*********************************************END OF FILE**********************/
