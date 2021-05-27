#include "myusart.h"


void MY_USART1_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART1_InitStruct;
	NVIC_InitTypeDef  NVIC_InitStruct;
	//1、串口、GPIOA时钟使能
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //GPIOA使能
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE); //串口使能
// <=> RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_USART1,ENABLE);
	
	//2、GPIOA端口设置
	GPIO_InitStruct.GPIO_Pin = USART1_GPIO_PIN_TX;	 //PA9
  	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;      
  	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 
  	GPIO_Init(GPIOA, &GPIO_InitStruct);		  

	GPIO_InitStruct.GPIO_Pin = USART1_GPIO_PIN_RX;	
  	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //浮空输入      
  	//GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 
  	GPIO_Init(GPIOA, &GPIO_InitStruct);	
	
	//3、串口参数初始化
	USART1_InitStruct.USART_BaudRate = bound;
	USART1_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART1_InitStruct.USART_Mode = USART_Mode_Rx|USART_Mode_Tx; //可接受可发送mode
	USART1_InitStruct.USART_Parity = USART_Parity_No;
	USART1_InitStruct.USART_StopBits = USART_StopBits_1;
	USART1_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1,&USART1_InitStruct);

	//4、开启中断并且初始化NVIC
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStruct);
	
	//5、使能串口
	USART_Cmd(USART1,ENABLE);
	
	//6、编写中断处理函数
}

/**
* @brief 发送函数
*/
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
* @brief 发送字符串
* @arg USARTx --- 串口号
* @arg *str --- 字符串
*/
void USART_SendString(USART_TypeDef *USARTx, char *str)
{
	while(*str != '\0')
	{
		USART_SendByte(USARTx, *str++);
	}
	while(USART_GetFlagStatus(USARTx,USART_FLAG_TC) == RESET);
}

/**
* @brief 接收函数
* @arg USARTx --> USART1 USART2...
* @return uint8_t字符
*/
uint8_t USART_ReceiveByte(USART_TypeDef* USARTx)
{
  while(USART_GetFlagStatus(USARTx,USART_FLAG_RXNE) == RESET);
  /* Receive Data */
  return (uint8_t)USART_ReceiveData(USARTx);
}

/**
*	@brief 串口中断函数
*/
void USART1_IRQHandler()
{
	u16 temp;
	/*USART_GetITStatus ==1 可以读取数值
						==0 不可读取数值 */
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		temp = USART_ReceiveData(USART1);
	}
	USART_SendByte(USART1,temp);  //将接收的数据通过串口再次发送
}

//为支持printf函数且不使用微库
#pragma import(__use_no_semihosting)                           
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;        
_sys_exit(int x) 
{ 
	x = x; 
} 

//重定向fputc(c标准函数)  
int fputc(int ch, FILE *f)
{      
	USART_SendData(USART1, (uint8_t) ch);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
	return (ch);
}
//重定向fputc(c标准函数) 
int fgetc(FILE *f)
{      
	while(USART_GetFlagStatus(USART1,USART_FLAG_RXNE) == RESET);
	return (int)USART_ReceiveData(USART1);
}











