#ifndef __DAC8563_H
#define __DAC8563_H
#include "sys.h"
 
#define LDAC PBout(3)   //DAC数据更新
#define CLR  PBout(4)   //异步清输入
#define SYNC PBout(5)   //电平触发控制输入
#define SCLK PBout(6)   //串行时钟输入
#define DIN  PBout(7)   //24bits串行数据输入
/*通道选择*/
#define Channel_DAC_A  0x18
#define Channel_DAC_B  0x19
#define Channel_DAC_AB 0x1F
/*增益、掉电等命令*/
#define DAC_B_Gain_2_A_Gain_2 0x020000 //Normal Default
#define DAC_B_Gain_2_A_Gain_1 0x020001
#define DAC_B_Gain_1_A_Gain_2	0x020002
#define DAC_B_Gain_1_A_Gain_1	0x020003 //Power_Down Default
#define Reset_All_Regs   0x280001
#define PowerUp_DAC_A_B  0x200003
#define Enable_Inter_Ref 0x380001    //内部参考电压是2.5v | 
#define Diable_Inter_Ref 0x380000 //Disable Internal Reference
#define Max_Dac_Data 65535 //输出电压为-10V

void DAC8563_GPIO_Init(void);
void DAC8563_Write_Command_Data(u8 command,u16 data);
void DAC8563_Init(void);
void DAC8563_SetVoltage(u8 DAC_Channel,u16 Dac_Data);
#endif

