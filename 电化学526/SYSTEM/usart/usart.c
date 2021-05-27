#include "usart.h"	  
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif
 
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

/*使用microLib的方法*/
 /* 
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}
*/
 
#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
//u8 usartRecBuffer[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
////接收状态
////bit15，	接收完成标志
////bit14，	接收到0x0d
////bit13~0，	接收到的有效字节数目
//u16 USART_RX_STA=0;       //接收状态标记	  
u8 CtrCode;   //控制码
u16 CtrNumber; //控制数
u8 RecData[RecLen];    //接收数据
u16 ORENum=0;
uint8_t connect_flag=0;
uint8_t wifi_connect_flag=0;
uint8_t wifi_disconnect_flag=0;
extern float   ads1220_ch1_voltage_value;                          //通道1存储的电压值
extern float   ads1220_ch2_voltage_value;   
char sdata[40];
extern u16 dataptr,dataptr2;
extern u16 CtrNumberForUart;

float DutyCycle,Frequency;
  
void uart_init( u32 bound){
 GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART1_InitStruct;
	NVIC_InitTypeDef  NVIC_InitStruct;
	//1????GPIOA????
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE); 
// <=> RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_USART1,ENABLE);
	
	//2?GPIOA????
	GPIO_InitStruct.GPIO_Pin = USART1_GPIO_PIN_TX;	 //PA9
  	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;      
  	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 
  	GPIO_Init(GPIOA, &GPIO_InitStruct);		  

	GPIO_InitStruct.GPIO_Pin = USART1_GPIO_PIN_RX;	
  	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;       
  	//GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 
  	GPIO_Init(GPIOA, &GPIO_InitStruct);	
	
	USART1_InitStruct.USART_BaudRate = bound;
	USART1_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART1_InitStruct.USART_Mode = USART_Mode_Rx|USART_Mode_Tx; //??????mode
	USART1_InitStruct.USART_Parity = USART_Parity_No;
	USART1_InitStruct.USART_StopBits = USART_StopBits_1;
	USART1_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1,&USART1_InitStruct);


	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStruct);
	
	USART_Cmd(USART1,ENABLE);
	
}

/**
* @brief 发送函数
*/
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
	/* 发送一个字节数据到USART */
	USART_SendData(pUSARTx,ch);
		
	/* 等待发送数据寄存器为空 */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}

void USART_SendByte(USART_TypeDef* USARTx, uint16_t Data)
{
  /* Check the parameters */
  assert_param(IS_USART_ALL_PERIPH(USARTx));
  assert_param(IS_USART_DATA(Data)); 
    
  /* Transmit Data */
  USARTx->DR = (Data & (uint16_t)0x01FF);
	while(USART_GetFlagStatus(USARTx,USART_FLAG_TXE) == RESET);
}

/**
*	@brief 发送N字节
*/
void Usart_SendNByte(USART_TypeDef * pUSARTx,int msglen,uint8_t *sendBuffer)
{
	int i=0;
	for(i=0;i<msglen;i++)
	{
		Usart_SendByte(USART1,sendBuffer[i]);
	}
}


void USART1_IRQHandler()
{
	u8 i; u16 j;
//	u16 temp;
	//u8 Res;
//#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
//	OSIntEnter();    
//#endif
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)					//串口1接收中断
	{	
		//接收全部数据：6位控制命令/其他位数命令
		for (i=0;i<RecLen;i++)
		{
			j=0;
			while(USART_GetFlagStatus(USART1,USART_IT_RXNE)==RESET)  	//未接收到数据则进入等待循环
			{
				j++;
				if (j>1000) 
				{
					break;               	//  等待超时则退出等待循环
				}	
			}			
			if (j>1000) 
			{
				CtrCode='E';	//表示Error
				CtrNumber=i;
				break;//否则跳出for循环
			}
			else
			{
				RecData[i]=USART_ReceiveData(USART1);
			}
		}
		
		if (i<RecLen)									//非6位数据处理
		{
			//收到 c 连接mqtt
			if(RecData[0] == 'c')
			{
				wifi_connect_flag=1;
				wifi_disconnect_flag=0;
				CtrCode = ' ';
				dataptr2=dataptr=0;
//				RecData[0] = '\0';
			}
			else if(RecData[0] == 's')
			{
				wifi_disconnect_flag=1;
				wifi_connect_flag=0;
				connect_flag=0;
				CtrCode = ' ';
				dataptr2=dataptr=0;
//				RecData[0] = '\0';
			}
			return ;
		}
		
		if(connect_flag==1)						//*若已联网
		{
			//解析命令
			CtrCode='P';
			if(dataptr)
			{
				CtrNumber=(u16)((DATA_BUF[2])*1000+(DATA_BUF[3])*100+(DATA_BUF[4])*10+(DATA_BUF[5]));	
			}
		}
		else													//若未联网
		{
			if (i==RecLen)
			{
				
				if ((((RecData[0]>64)&&(RecData[0]<91))||((RecData[0]>96)&&(RecData[0]<123)))&& //判断必须是大或小写字母！ (RecData[1]<58)&&(RecData[1]>47)&&
						(RecData[2]<58)&&(RecData[2]>47)&&
						(RecData[3]<58)&&(RecData[3]>47)&&(RecData[4]<58)&&(RecData[4]>47))    //判断必须是数字!
				{
					CtrCode=RecData[0];
					CtrNumber=(u16)((RecData[2]-0x30)*1000+(RecData[3]-0x30)*100+(RecData[4]-0x30)*10+(RecData[5]-0x30));	
					CtrNumberForUart = CtrNumber;
				}
				else 
				{
					CtrCode='A';	//表示Error
					CtrNumber=1000+i;
//					printf("AAAA");
				}
			}
		}
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);   //清除中断标志位
		

	}
	//过载错误,非常重要！20201103，当接收数据字节数超过时，如果没有改判断，则在中断中出不来！
	else if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)  
   {  
		j=USART1->SR;      //先读USART_SR；再读USART_DR，清除中断标志位（先读后读顺序好像不影响！20201113）
		j=USART1->DR; 
		ORENum++;         //出错计数
		CtrCode='E';	//表示Error
		CtrNumber=2000+ORENum;
		if (ORENum==999)   ORENum=0;	
	}
	 
	#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
}
#endif	

