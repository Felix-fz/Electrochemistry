#ifndef __USART2_H
#define __USART2_H	 
#include "sys.h"  

#define USART2_MAX_RECV_LEN		512				//最大接收缓存字节数
#define USART2_MAX_SEND_LEN		1024				//最大发送缓存字节数
#define USART2_RX_EN 			1					//0,不接收;1,接收.

extern u8  USART2_RX_BUF[USART2_MAX_RECV_LEN]; 		//接收缓冲,最大USART2_MAX_RECV_LEN字节
extern u8  USART2_TX_BUF[USART2_MAX_SEND_LEN]; 		//发送缓冲,最大USART2_MAX_SEND_LEN字节
extern u16 USART2_RX_STA;   						//接收数据状态

void USART2_Init(u32 bound);				//串口2初始化 
void TIM4_Set(u8 sta);
void TIM4_Init(u16 arr,u16 psc);
void UART_DMA_Config_Tx(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar);
void UART_DMA_Config_Rx(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar, u16 cndtr);
void UART_DMA_Enable(DMA_Channel_TypeDef*DMA_CHx,u8 len);
void u2_printf(char* fmt, ...);
void MYDMA_Enable_esp(DMA_Channel_TypeDef*DMA_CHx);
static void buf_chn2(void);
#endif













