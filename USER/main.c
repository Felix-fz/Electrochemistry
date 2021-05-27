#include "sys.h"

u8 status = 0;

uint8_t sendBuffer[20]={0}, order=0,i=0;

extern u8 CtrCode;
extern u16 CtrNumber;
u16 CtrNumberForUart;
u16 DACInData;       //DAC的数字输入端赋值
extern char sdata[];

extern uint8_t wifi_connect_flag;
extern uint8_t wifi_disconnect_flag;
extern uint8_t connect_flag;
extern u16 dataptr;
u16 dataptr2 = 0;


int main(void)
{	 

	delay_init();	    	 //延时函数初始化	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级

	uart_init(115200);	 	//串口初始化为115200	
	
	ADS1220_SPI2_Init();           //ADS1220初始化
	ADS1220_Setting(1);            //ADS1220配置寄存器channel2
	
	DAC8563_Init();
	USART2_Init(115200);  //串口2初始化为115200	
	
	esp_send_cmd_IT("AT+MQTTCLEAN=0\r\n", 200);
	esp_send_cmd_IT("AT+CWQAP\r\n", 200);
	printf("Clear WIFI!");
	
	while(1)
  {
		if(wifi_connect_flag)
		{
			wifi_connect_flag=0;
			mqtt_connect();
			printf("mqtt connected");
			//Publish_MSG("\"test_data\"","hello");
			connect_flag=1;
		}
		else if(wifi_disconnect_flag)
		{
			wifi_disconnect_flag=0;
			esp_send_cmd_IT("AT+MQTTCLEAN=0\r\n", 200);
			esp_send_cmd_IT("AT+CWQAP\r\n", 200);
			printf("Clear WIFI!");
		}
		
		

		if(dataptr>0)  // 存在cmd
		{
			if(DATA_BUF[dataptr2]>=0)
			{
				CtrCode = 'P';
				CtrNumber = DATA_BUF[dataptr2];
			}
			else
			{
				CtrCode = 'N';
				CtrNumber = -DATA_BUF[dataptr2];
			}
		}
		
		switch (CtrCode)
		{
			case'P'://DAC控制  // 0 65535  -32768  0  32768
 				DACInData = (u16)(CtrNumber*3.277+32767);	
				DAC8563_SetVoltage(Channel_DAC_AB,DACInData);
//				printf("%d",DACInData);
				delay_ms(3);
				ADS1220_Channel_Read_Data();
				break;
			case'N'://DAC控制
				DACInData = (u16)(32768-CtrNumber*3.277);	
				DAC8563_SetVoltage(Channel_DAC_AB,DACInData);
				delay_ms(3);
				ADS1220_Channel_Read_Data();
				CtrCode = ' ';
				break;
			case'E'://出错代码输出
				printf("%c%d\r\n",CtrCode,CtrNumber);
				CtrCode = ' ';
				break;
			case'S'://连接串口应答
				printf("J");
				CtrCode = ' ';
				break;
		}
		
		}
		
}		
 
 
