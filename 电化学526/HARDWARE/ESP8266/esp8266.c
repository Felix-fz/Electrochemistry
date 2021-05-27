#include "esp8266.h"

//返回值:0,没有得到期待的应答结果
//    其他,期待应答结果的位置(str的位置)
u8* esp_check_cmd(char *str)
{
	USART2_RX_BUF[USART2_RX_STA]=0;      //添加结束符
	return (u8*)strstr((const char*)USART2_RX_BUF,(const char*)str);
}
u8* esp_check_cmd2(char *str)
{
	USART2_RX_BUF[USART2_RX_STA+1]=0;	   //添加结束符
	return (u8*)strstr((const char*)(USART2_RX_BUF+25),(const char*)str);
}

//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
extern u16 USART2_RX_STA;
unsigned char isok = 0;
u8 esp_send_cmd_IT(char *cmd,u16 waittime)
{
	isok = 0;
	u2_printf("%s\r\n",cmd);					//发送命令
	printf("%s\r\n",cmd);
	while(--waittime)									//等待倒计时
	{
		delay_ms(1);
		if(isok)
		{
			break;												//得到有效数据 ack
		}
	}
	if(waittime==0)return 1; 
	return 0;
} 

int mqtt_connect(void)
{
	while(esp_send_cmd_IT("AT+CWJAP=\"iPhone\",\"19990929\"",2000))
	{
		printf("connecting wifi/r/n");
	}
	printf(".");
	while(esp_send_cmd_IT("ATE0",200));
	while(esp_send_cmd_IT("AT+MQTTUSERCFG=0,1,\"ESP\",\"esp\",\"1234567890\",0,0,\"\"",500));
	delay_ms(1000);
	while(esp_send_cmd_IT("AT+MQTTCONN=0,\"124.71.235.8\",1883,0",1000));
	while(esp_send_cmd_IT("AT+MQTTSUB=0,\"sub_msg\",0",500));
	printf(".");
	delay_ms(500);
	USART2_RX_STA = 0;
	return 1;
}

char pub_flag = 0;
u8 cnt = 0;

int Publish_MSG(char *topic,char *arr)
{
	char buf[80];
	cnt = 20;
	USART2_RX_STA = 0;
	sprintf(buf,"AT+MQTTPUB=0,%s,\"%s\",0,0",topic,arr);
	while(esp_send_cmd_IT(buf,200))
	{
		cnt--;
		pub_flag=1;//发送失败一直是1，一直尝试
		if(cnt<=0)break;
	}
	pub_flag=0;
	USART2_RX_STA = 0;
	return 1;
}

//4位char数字转成int数字
static int str4_to_int(unsigned char * str)
{
	int r = 0;
	if(str[0])r += (int)(str[0] - 0x30) * 1000;
	if(str[1])r += (int)(str[1] - 0x30) * 100 ;
	if(str[2])r += (int)(str[2] - 0x30) * 10  ;
	if(str[3])r += (int)(str[3] - 0x30) * 1   ;
	return r;
}

//extern unsigned char status;	//0 stop  1 start
int DATA_BUF[2048];							  //命令缓存数组
u16 dataptr = 0;									//命令缓存数组计数器
extern u8 CtrCode;

void Subscribe_MSG_IT(void)
{
	if(esp_check_cmd2("P:"))
	{
		//P命令
//		CtrCode = 'P';
		DATA_BUF[dataptr++] = str4_to_int(USART2_RX_BUF+29) * 1;
	}
	else if(esp_check_cmd2("N:"))
	{
		//N命令
//		CtrCode = 'N';
		DATA_BUF[dataptr++] = str4_to_int(USART2_RX_BUF+29) * (-1);
	}
//	else if(esp_check_cmd2("start"))
//	{
//		//接收到start命令
//		status = 1;
//	}
//	else if(esp_check_cmd2("stop"))
//	{
//		//接收到stop命令
//		status = 0;	
//	}
	else 
		return ;
	USART2_RX_STA = 0;
}
