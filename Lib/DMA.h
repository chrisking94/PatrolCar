#ifndef _DMA_H_
#define _DMA_H_

#include "common.h"

void DMA_init(void);
void DMA_Restart(DMA_Channel_TypeDef* CHx,u16 NumberOfDataToTransfer);//启动也可以使用这个函数

#endif
