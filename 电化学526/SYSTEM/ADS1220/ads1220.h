#ifndef _ADS1220_H
#define _ADS1220_H

#include "sys.h"

//#define DRDY     GPIO_Pin_6      //PC6

#define DRDY     GPIO_Pin_8   //PA8
#define CS_Pin   GPIO_Pin_12     //PB12
#define SCK_Pin  GPIO_Pin_13     //PB13
#define MISO_Pin GPIO_Pin_14     //PB14
#define MOSI_Pin GPIO_Pin_15     //PB15
#define ADC_DRDY GPIO_ReadInputDataBit(GPIOC,DRDY)

//ADS1220命令表
#define ADC_CMD_RESET      0X06             //复位器件                      
#define ADC_CMD_START      0X08             //启动或重启转换                
#define ADC_CMD_POWERDOWN  0X02             //进入掉电模式                  
#define ADC_CMD_RDATA      0X10             //通过命令读取数据             
#define ADC_CMD_RREG       0X20             //读取nn 寄存器（起始地址：rr） 
#define ADC_CMD_WREG       0X40             //写入nn 寄存器（起始地址：rr） 
//操作数：rr = 配置寄存器（00 至11），nn = 字节数– 1（00 至11）以及x = 无关值。

//ADS寄存器配置表


//引脚控制
#define ADC_CS_0    GPIO_ResetBits(GPIOB, GPIO_Pin_12)    //PB12
#define ADC_CS_1    GPIO_SetBits(GPIOB, GPIO_Pin_12)
#define ADC_SCLK_0  GPIO_ResetBits(GPIOB, GPIO_Pin_13)
#define ADC_SCLK_1  GPIO_SetBits(GPIOB, GPIO_Pin_13)

//ads1220读取并存储数据
//void ADS1220_Read_Data(void);
void ADS1220_SPI2_Init(void);
void ADS1220_Setting(u8 ch);//ADC初始化配置
double ADS1220_Get_Voltage_Conversion_Data(void);
void ADS1220_Channel_Read_Data(void);
#endif

