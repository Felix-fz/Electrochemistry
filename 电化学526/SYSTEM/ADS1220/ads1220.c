#include "ads1220.h"

float   ads1220_storage_10_value[10];                       //ads存储10次的数据
float   ads1220_Temporary_voltage_value_sum = 0.0;          //临时电压存储的值累加变量
u8      ads1220_sw_count = 0;                               //ads数据运行数量。10个值清零一次。
u8      ads1220_cur_ch = 1;                                 //ads当前运行的通道
float  ads1220_ch1_voltage_value;                          //通道1存储的电压值
float  ads1220_ch2_voltage_value;                          //通道2存储的电压值
double Vref = 2.52; //参考电压值

/**
*	@breif SPI2写入多字节
*	@param u8 *write_data --- 写入内容
*				 u8 *read_data  --- 接收回应
*        u8 length  --- 发送字节长度
**/
void SPI2_Send_Mul_Byte(u8 *write_data,u8 *read_data, u8 length)
{
    u8 i;
    ADC_CS_0;     //CS拉低
    delay_us(50);
    for (i=0; i<length; i++)   
    {
        read_data[i] = SPI2_Send_Byte(write_data[i]);
    }
    delay_us(50);
    ADC_CS_1;    //CS置高
}



/**
*	@brief ADS1220复位操作
*	@param none
*/
void ADS1220_Reset(void)
{
    u8 temp_cmd = ADC_CMD_RESET;    //复位命令
    u8  cur_rec_data[10];           //当前接收到数据，无用，但是存储一下
    SPI2_Send_Mul_Byte(&temp_cmd, cur_rec_data, 1);
}


/**
*	@breif ADS1220启动
*	@param none
*/
void ADS1220_Start(void)
{
    u8 temp_cmd = ADC_CMD_START;
    u8  cur_rec_data[10];          //当前接收到数据，无用，但是存储一下
    SPI2_Send_Mul_Byte(&temp_cmd, cur_rec_data, 1);
}


/**
*	@brief ADC1220进入掉电模式
*	@param none
*/
void ADS1220_Powerdown(void)
{
    u8 temp_cmd = ADC_CMD_POWERDOWN;
    u8  cur_rec_data[10];            //当前接收到数据，无用，但是存储一下
    SPI2_Send_Mul_Byte(&temp_cmd, cur_rec_data, 1);
}


/**
*	@brief ADC1220通过命令读数据
*/
void ADS1220_Rdata(void)
{
    u8 temp_cmd = ADC_CMD_RDATA;
    u8  cur_rec_data[10];               //当前接收到数据，无用，但是存储一下
    SPI2_Send_Mul_Byte(&temp_cmd, cur_rec_data, 1);
}



/**
*	@brief ADC1220写入多个寄存器值
*	@param  u8 *writeValues --- 写入寄存器的内容
*				  u8 startReg --- 开始寄存器
*					u8 length --- 写入长度
*/
void ADS1220_Write_Regs(u8 *writeValues, u8 startReg, u8 length)
{
    u8 temp_send_data[5] = {0, 0, 0, 0, 0};
    u8 temp_i;
    u8  cur_rec_data[10];           //当前接收到数据，无用，但是存储一下
    temp_send_data[0] += ADC_CMD_WREG| ((startReg << 2) & 0x0c) |((length - 1) & 0x03);
    for (temp_i=0; temp_i<length; temp_i++)
    {
        temp_send_data[temp_i+1] = writeValues[temp_i];
    }
    SPI2_Send_Mul_Byte(temp_send_data, cur_rec_data, length+1);
}



/**
*	@breif ADC 读取多个寄存器值
*	@param	u8 *recValues  --- 接收值
*					u8 startReg  --- 开始寄存器
*					u8 length --- 长度
*/
void ADS1220_Read_Regs(u8 *recValues, u8 startReg, u8 length)
{
    u8 temp_rec_data[5] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF};
    temp_rec_data[0] = ADC_CMD_RREG | ((startReg << 2) & 0x0c) |((length - 1) & 0x03);
    SPI2_Send_Mul_Byte(temp_rec_data, recValues, length+1);
}


/**
*	@brief ADC 读取未转换的数据（无用）
*	@param u8 *conversionData --- 未转换数据
*/
void ADS1220_Get_Conversion_Data(u8 *conversionData)
{
    u8 outData[3] = {0xff, 0xff, 0xff};
    SPI2_Send_Mul_Byte(outData, conversionData, 3);    // 3 Bytes of Conversion Data
}






/**
*	@brief ADS1220初始化
*	@param none
*/
void ADS1220_SPI2_Init(void)  
{
    
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    /* pins: SCK 888.8ns*/
    GPIO_InitStructure.GPIO_Pin = SCK_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* pins: MISO */
    GPIO_InitStructure.GPIO_Pin = MISO_Pin;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* pins: MOSI */
    GPIO_InitStructure.GPIO_Pin = MOSI_Pin;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*CS pin */
    GPIO_InitStructure.GPIO_Pin = CS_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
   
		ADC_CS_1;           //片选拉高
		SPIx_Config(SPI2);	/* SPI2 configuration */
    //ADS1220_DRDY_GPIO_Init();  //初始化ADS1220 DRDY
		
		/* ADC DRTY  PC6*/
    GPIO_InitStructure.GPIO_Pin = DRDY;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}


/**
*	@brief ADC1220初始化配置
* @param u8 ch --- 通道
*/
void ADS1220_Setting(u8 ch)
{
	/****************************************
	*reg0：通道和增益设置 |MUX[7:4]|GAIN[3:1]|PGA_BYPASS[0]| 
	*reg1: 转换速度配置   |DR[7:5] |MODE[1:0]|CM[2]|TS[1]|BCS[0]|
	*reg2：内部参考电压   |Vref[1:0]| 50/60[1:0]|PSW|IDAC[2:0]|
	*reg3：               |I1MUX[2:0]|I2MUX[2:0]|DRDYM[1]|0|
	*数据格式：{reg0,reg1,reg2,reg3|     
	*****************************************/
    u8  temp_write_data_arry1[4] = {       
			//0x01,// AINP=AIN0,AINN=AIN1
			0x91,  //AINP=AIN1, AINN=AVSS
			//0XA1,  //寄存器0，复用1000：AINP = AIN2，AINN = AVSS   GAIN=1    禁用和旁路内部低噪声PGA
      0XA4, //寄存器1，101 = 600SPS   normal  1：连续转换模式
						 //0x10,  //使用内部参考电压2.04V FIR滤波
			0X58, //寄存器2，01：使用专用REFP0 和REFN0 输入选择的外部基准电压 FIR滤波0101_1000
      0X00  //寄存器3，
                                    };
    u8  temp_write_data_arry2[4] = {    
			0XA1,       //寄存器0，复用1010：AINP = AIN2，AINN = AVSS       禁用和旁路内部低噪声PGA
      0XA4,       //寄存器1，101 = 600SPS     1：连续转换模式
      //0X40,       //寄存器2，01：使用专用REFP0 和REFN0 输入选择的外部基准电压
			0x58, //
      0X00        //寄存器3，
                                    };
    u8  temp_read_data_arry[5];

    ADS1220_Reset();
    delay_ms(10);
    if(1 == ch)
    {
        ADS1220_Write_Regs(temp_write_data_arry1, 0, 4);   
    }
    else
    {
        ADS1220_Write_Regs(temp_write_data_arry2, 0, 4);
    }
    delay_ms(10);
    ADS1220_Read_Regs(temp_read_data_arry, 0, 4);
    //printf("reg data%d: %02X %02X %02X %02X\r\n", ch, temp_read_data_arry[1], temp_read_data_arry[2], temp_read_data_arry[3], temp_read_data_arry[4]);
    delay_ms(10);
    ADS1220_Start();    //启动ADS1220
}

/**
*	@brief ADC读取电压数据
*	@param none
*/
double ADS1220_Get_Voltage_Conversion_Data(void)
{
    u8      temp_send_data[3] = {0xff, 0xff, 0xff}; //写入内容
    u8      temp_hex_data[3];  //ADS读取到值
    double  temp_double_data;
		
    SPI2_Send_Mul_Byte(temp_send_data, temp_hex_data, 3);    // 3 Bytes of Conversion Data
		//printf("%x,%x,%x",temp_hex_data[0],temp_hex_data[1],temp_hex_data[2]);
		
		/**获得值转换算法**/
		temp_double_data = (((long)temp_hex_data[0] << 16) + ((long)temp_hex_data[1] << 8) + ((long)temp_hex_data[2])); 
		temp_double_data = (temp_double_data * Vref)/8388608;
		//printf("voltage_data = %lf\r\n", (2*temp_double_data-2.514));
		return temp_double_data;
}


/**
*	@brief 两通道并行
*/

extern u8 CtrCode;
extern u16 CtrNumber;
extern u16 CtrNumberForUart;
extern char sdata[];

extern uint8_t wifi_flag;
extern uint8_t connect_flag;
extern u16 dataptr,dataptr2;

void ADS1220_Channel_Read_Data(void)
{
	u8 i;
	if(ads1220_cur_ch == 1)         //当前通道为channel 1
	{
		ads1220_ch1_voltage_value=0;
		for(i=0;i<3;i++)
		{
			ads1220_ch1_voltage_value+=(2*ADS1220_Get_Voltage_Conversion_Data()-2.514)*10000;
			delay_ms(2);
		
		}
		if(ads1220_ch1_voltage_value > 0)
		{
			if(connect_flag==0)
			{
				printf("{P:%d,P:%d}\r\n" , CtrNumberForUart, (int)(ads1220_ch1_voltage_value/3));
				CtrCode = ' ';
			}
			else
			{
				if(dataptr>0&&dataptr2<=dataptr)
				{
					sprintf(sdata, "{P:%04d\\,P:%d\\}" ,DATA_BUF[dataptr2++],(int)(ads1220_ch1_voltage_value/3));//命令+数据
					DATA_BUF[dataptr2-1] = 0;
					Publish_MSG("\"test_data\"",sdata);
					if(dataptr2==dataptr)
					{
						dataptr2=dataptr=0;
						CtrCode = ' ';
					}
				}
			}
		}
		else
		{
			if(connect_flag==0)
			{
				if(CtrNumberForUart>0)
					printf("{P:%d,N:%d}\r\n" , CtrNumberForUart, (int)(-ads1220_ch1_voltage_value/3));
				else
					printf("{N:%d,N:%d}\r\n" , CtrNumberForUart, (int)(-ads1220_ch1_voltage_value/3));
				CtrCode = ' ';
			}
			else
			{
				if(dataptr>0&&dataptr2<=dataptr)
				{
					if(DATA_BUF[dataptr2]>0)
						sprintf(sdata, "{P:%04d\\,N:%d\\}" ,DATA_BUF[dataptr2++],(int)(-ads1220_ch1_voltage_value/3));//命令+数据
					else
						sprintf(sdata, "{N:%04d\\,N:%d\\}" ,DATA_BUF[dataptr2++]*(-1),(int)(-ads1220_ch1_voltage_value/3));//命令+数据
					DATA_BUF[dataptr2-1] = 0;
					Publish_MSG("\"test_data\"",sdata);
					if(dataptr2==dataptr)
					{
						dataptr2=dataptr=0;
						CtrCode = ' ';
					}
				}
			}
		}
	}
	else if(ads1220_cur_ch == 2)   //当前通道为channel 2
	{
		ads1220_cur_ch = 1;
		ads1220_ch2_voltage_value = (2*ADS1220_Get_Voltage_Conversion_Data()-2.514)*10000;
		ADS1220_Setting(1);				
		//printf("ch2:  %lf\r\n", ads1220_ch2_voltage_value);
	}
}


