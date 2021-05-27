#include "delay.h"
#include "usart2.h"
#include "stdarg.h"	 	 
#include "sys.h"

#define Subscribe_IT

//串口发送缓存区 	
__align(8) u8 USART2_TX_BUF[USART2_MAX_SEND_LEN];
//串口接收缓存区 	
u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 				//接收缓冲,最大USART2_MAX_RECV_LEN个字节.
u8 USART2_RX_BUF_1[USART2_MAX_RECV_LEN];
u8 USART2_RX_BUF_2[USART2_MAX_RECV_LEN];
u8 rec_by = 0;
extern unsigned char status, isok;

u16 USART2_RX_STA=0;   	 
void USART2_IRQHandler(void)
{
	if(USART_GetFlagStatus(USART2, USART_FLAG_IDLE) != RESET)//接收完成
	{
		USART_ClearFlag(USART2,USART_IT_IDLE);
		USART2->SR;
    USART2->DR; 
		DMA_Cmd(DMA1_Channel6,DISABLE);
		USART2_RX_STA = USART2_MAX_RECV_LEN - DMA_GetCurrDataCounter(DMA1_Channel6);
		
		buf_chn2();//双缓存切换
		MYDMA_Enable_esp(DMA1_Channel6);
		
		if(esp_check_cmd("OK"))
		{
			isok = 1; 
			USART2_RX_STA = 0;
//			MYDMA_Enable_esp(DMA1_Channel6);
			return;
		}
		
		
//		UsartPrintf(USART1,"\r\n@");
//		UsartPrintf(USART1,(char*)USART2_RX_BUF);
//		UsartPrintf(USART1,"@\r\n");
		#ifdef Subscribe_IT
		Subscribe_MSG_IT();
		#endif
		memset(USART2_RX_BUF,0,USART2_RX_STA);
//		MYDMA_Enable_esp(DMA1_Channel6);
	}
}   

void USART2_Init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);

 	USART_DeInit(USART2);  //复位串口2
		 //USART2_TX   PA.2
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA2
   
    //USART2_RX	  PA.3
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA3
	
	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  
	USART_Init(USART2, &USART_InitStructure); //初始化串口2
  
	
	USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE);  	//使能串口2的DMA发送
	UART_DMA_Config_Tx(DMA1_Channel7,(u32)&USART2->DR,(u32)USART2_TX_BUF);//DMA1通道7,外设为串口2,存储器为USART2_TX_BUF 
	
	USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE); //使能串口2的DMA接收
	UART_DMA_Config_Rx(DMA1_Channel6,(u32)&USART2->DR,(u32)USART2_RX_BUF_1, USART2_MAX_RECV_LEN);
	
	USART_Cmd(USART2, ENABLE);                    //使能串口 	
	
	//使能接收中断
//  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断   USART_IT_IDLE
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
//	TIM4_Init(99,7199);		//1ms中断
	USART2_RX_STA=0;		//清零
//	TIM4_Set(0);			//关闭定时器4


}
//串口2,printf 函数
//确保一次发送数据不超过USART2_MAX_SEND_LEN字节
void u2_printf(char* fmt,...)  
{  
	va_list ap;
	va_start(ap,fmt);
	vsprintf((char*)USART2_TX_BUF,fmt,ap);
	va_end(ap);
	while(DMA_GetCurrDataCounter(DMA1_Channel7)!=0);	//等待通道7传输完成   
	UART_DMA_Enable(DMA1_Channel7,strlen((const char*)USART2_TX_BUF)); 	//通过dma发送出去
}

//发送用DMA
void UART_DMA_Config_Tx(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar)
{
	DMA_InitTypeDef DMA_InitStructure_esp;
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输
  DMA_DeInit(DMA_CHx);   //将DMA的通道1寄存器重设为缺省值
	DMA_InitStructure_esp.DMA_PeripheralBaseAddr = cpar;  //DMA外设基地址
	DMA_InitStructure_esp.DMA_MemoryBaseAddr = cmar;  //DMA内存基地址
	DMA_InitStructure_esp.DMA_DIR = DMA_DIR_PeripheralDST;  //数据传输方向，从内存读取发送到外设
	DMA_InitStructure_esp.DMA_BufferSize = 0;  //DMA通道的DMA缓存的大小
	DMA_InitStructure_esp.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA_InitStructure_esp.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA_InitStructure_esp.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //数据宽度为8位
	DMA_InitStructure_esp.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
	DMA_InitStructure_esp.DMA_Mode = DMA_Mode_Normal;  //工作在正常缓存模式
	DMA_InitStructure_esp.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
	DMA_InitStructure_esp.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA_CHx, &DMA_InitStructure_esp);  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器	
} 

//接收命令用DMA
static u16 DMA1_MEM_LEN;
DMA_InitTypeDef DMA_InitStructure_esp;

void UART_DMA_Config_Rx(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar, u16 cndtr)
{
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输
  DMA_DeInit(DMA_CHx);   //将DMA的通道1寄存器重设为缺省值
	DMA1_MEM_LEN=cndtr;
	DMA_InitStructure_esp.DMA_PeripheralBaseAddr = cpar;  //DMA外设基地址
	DMA_InitStructure_esp.DMA_MemoryBaseAddr = cmar;  //DMA内存基地址
	DMA_InitStructure_esp.DMA_DIR = DMA_DIR_PeripheralSRC;  //数据传输方向，从内存读取发送到外设
	DMA_InitStructure_esp.DMA_BufferSize = cndtr;  //DMA通道的DMA缓存的大小
	DMA_InitStructure_esp.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA_InitStructure_esp.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA_InitStructure_esp.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //数据宽度为8位
	DMA_InitStructure_esp.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
	DMA_InitStructure_esp.DMA_Mode = DMA_Mode_Normal;  //工作在正常缓存模式
	DMA_InitStructure_esp.DMA_Priority = DMA_Priority_High; //DMA通道 x拥有中优先级 
	DMA_InitStructure_esp.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA_CHx, &DMA_InitStructure_esp);  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器	
	//DMA_Cmd(DMA_CHx, ENABLE); //使能USART1 TX DMA1 所指示的通道 
	MYDMA_Enable_esp(DMA1_Channel5);//开始一次DMA传输！
} 

void DMA2_Channel5_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA2_IT_TC2))
    {
        DMA_ClearITPendingBit(DMA2_IT_GL2); //清除全部中断标志
    }
}

//开启一次DMA传输
void UART_DMA_Enable(DMA_Channel_TypeDef*DMA_CHx,u8 len)
{
	DMA_Cmd(DMA_CHx, DISABLE );  //关闭 指示的通道        
	DMA_SetCurrDataCounter(DMA_CHx,len);//DMA通道的DMA缓存的大小	
	DMA_Cmd(DMA_CHx, ENABLE);           //开启DMA传输
}	   

void MYDMA_Enable_esp(DMA_Channel_TypeDef*DMA_CHx)
{ 
DMA_Cmd(DMA_CHx, DISABLE ); //关闭USART1 TX DMA1 所指示的通道 
DMA_SetCurrDataCounter(DMA_CHx,DMA1_MEM_LEN);//从新设置缓冲大小,指向数组0
DMA_Cmd(DMA_CHx, ENABLE); //使能USART1 TX DMA1 所指示的通道 
}   

static void buf_chn2(void)
{
	DMA_InitStructure_esp.DMA_BufferSize = USART2_MAX_SEND_LEN;
	if(rec_by==0)
	{
		rec_by=1;
		memcpy(USART2_RX_BUF,USART2_RX_BUF_1,USART2_RX_STA);    
    //memset(USART2_RX_BUF_1,0,USART2_MAX_SEND_LEN);
		DMA_InitStructure_esp.DMA_MemoryBaseAddr = (uint32_t)USART2_RX_BUF_2;         // 设置接收缓冲区首地址
		DMA_Init(DMA1_Channel6, &DMA_InitStructure_esp);               // 写入配置
	}
	else
	{
		rec_by=0;
		memcpy(USART2_RX_BUF,USART2_RX_BUF_2,USART2_RX_STA);
    //memset(USART2_RX_BUF_2,0,USART2_MAX_SEND_LEN);
		DMA_InitStructure_esp.DMA_MemoryBaseAddr = (uint32_t)USART2_RX_BUF_1;         // 设置接收缓冲区首地址
		DMA_Init(DMA1_Channel6, &DMA_InitStructure_esp);               // 写入配置    
	}
	DMA_Cmd(DMA1_Channel6, ENABLE);  
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 									 





















