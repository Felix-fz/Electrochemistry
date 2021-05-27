#ifndef __SPI2_H
#define __SPI2_H

#include "sys.h"
void SPIx_Config(SPI_TypeDef* SPIx);
u8 SPI2_Send_Byte(u8 byte);
u8 SPI2_Receive_Byte(void);

#endif
