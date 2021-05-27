#ifndef _DAC8552_h_
#define _DAC8552_h_

#include "sys.h"

#define DIN1 GPIO_SetBits(GPIOC,GPIO_Pin_11)
#define SCLK1 GPIO_SetBits(GPIOC,GPIO_Pin_10)
#define SYNC1  GPIO_SetBits(GPIOC,GPIO_Pin_9)


#define DIN0   GPIO_ResetBits(GPIOC,GPIO_Pin_11)
#define SCLK0  GPIO_ResetBits(GPIOC,GPIO_Pin_10)
#define SYNC0  GPIO_ResetBits(GPIOC,GPIO_Pin_9)



#define Channel_A       1
#define Channel_B       2
#define Channel_AB      3

void DAC8552_write(unsigned char commond, unsigned int date);
//void voltage_output(unsigned char channel, double vl2);
void voltage_output(unsigned char channel, unsigned int vl2);
void DAC8552_init(void);
void DAC_A(unsigned int data);
void DAC_B(unsigned int data);
void DAC_AB(unsigned int data_A,unsigned int data_B) ;
void write_8552(unsigned int data) ;


#endif
