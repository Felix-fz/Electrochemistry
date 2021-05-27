#include "DAC8563.h"

/**
*	@brief DAC8563写数据操作 （共24bits）
*       DAC856x DATA Input Register Format
* |   |Command | Address |    DATA   |
*  XX  C2 C1 C0 A2 A1 A0  D15 D14...D0   (DB23-DB0, X don't care)
*    高8bits是command 低16bits是data
*/
void DAC8563_Write_Command_Data(u8 command,u16 data)
{
	int i;
	SYNC = 1;
	delay_us(1);	
	SYNC = 0;   //SYN拉低时写数据有效，时钟下降沿写入寄存器
	SCLK=0;
	for(i=7;i>=0;i--)  //写入指令与地址
	{
		if((command&0x80)==0x80){DIN=1;}
    else{DIN=0;}
    delay_us(1);	
		
		SCLK = 1;
		delay_us(10);
		DIN = (command>>i)%2;
		delay_us(10);
		SCLK = 0;
		delay_us(10);
	}
	for(i=15;i>=0;i--) //写入相应的数据
	{
		if((data&0x8000)==0x8000){DIN=1;}
    else{DIN=0;}
		SCLK = 1;
		delay_us(10);
		DIN = (data>>i)%2;
		delay_us(10);
		SCLK = 0;
		delay_us(10);
	}
}



/**
*	@brief DAC8563初始化
*/
void DAC8563_Init(void)
{
		DAC8563_GPIO_Init(); //引脚初始化
	
		GPIO_SetBits(GPIOB,GPIO_Pin_5);  //SYNC置高
		GPIO_SetBits(GPIOB,GPIO_Pin_4);  //CLR置高
		
		GPIO_ResetBits(GPIOB,GPIO_Pin_6); //SCLK==0
		GPIO_ResetBits(GPIOB,GPIO_Pin_7); //DIN==0

	  /*配置DAC8563*/
		DAC8563_Write_Command_Data((Reset_All_Regs>>16),(Reset_All_Regs&0xFFFF));   //复位
		DAC8563_Write_Command_Data((PowerUp_DAC_A_B>>16),(PowerUp_DAC_A_B&0xFFFF)); //启动A和B
		DAC8563_Write_Command_Data((Enable_Inter_Ref>>16),(Enable_Inter_Ref&0xFFFF)); //选用内部参考电压
}

/**
*	@brief DAC8563电压设置  (Dac_Data/65536)*Gain*Vref
*/
void DAC8563_SetVoltage(u8 DAC_Channel,u16 Dac_Data) 
{
	if(DAC_Channel == Channel_DAC_A)
	{
		if(Dac_Data <= Max_Dac_Data)
			DAC8563_Write_Command_Data(Channel_DAC_A,Dac_Data);
		else
			DAC8563_Write_Command_Data(Channel_DAC_A,Max_Dac_Data);
	}
	if(DAC_Channel == Channel_DAC_B)
	{
		if(Dac_Data <= Max_Dac_Data)
			DAC8563_Write_Command_Data(Channel_DAC_B,Dac_Data);
		else
			DAC8563_Write_Command_Data(Channel_DAC_B,Max_Dac_Data);
	}
	if(DAC_Channel == Channel_DAC_AB)
	{
		if(Dac_Data <= Max_Dac_Data)
			DAC8563_Write_Command_Data(Channel_DAC_AB,Dac_Data);
		else
			DAC8563_Write_Command_Data(Channel_DAC_AB,Max_Dac_Data);
	}
	LDAC = 0;
	delay_us(100);
	LDAC = 1;
}

/**
*  @brief 配置引脚 PC0~PC4
*/
void DAC8563_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE);	//使能GPIOB时钟
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}


