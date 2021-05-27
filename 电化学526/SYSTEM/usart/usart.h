#ifndef __USART_H
#define __USART_H

#include "stdio.h"	
#include "sys.h" 

#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收

#define USART1_GPIO_PIN_TX GPIO_Pin_9  //PA9
#define USART1_GPIO_PIN_RX GPIO_Pin_10

#define RecLen 6
	  	
extern u8  usartRecBuffer[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	
extern int DATA_BUF[];
//如果想串口中断接收，请不要注释以下宏定义
void uart_init(u32 bound);
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch);
void USART_SendString(USART_TypeDef *USARTx, char *str);
void Usart_SendNByte(USART_TypeDef * pUSARTx,int msglen,uint8_t *sendBuffer);

#endif


