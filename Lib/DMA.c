#include "DMA.h"
#include "adc.h"

#ifdef DEBUG
	#include "usart.h"
#endif

#define DMA_SIZE_8_BIT 0
#define DMA_SIZE_16_BIT 1
#define DMA_SIZE_32_BIT 2

typedef struct
{
	u32 Priority;//Priority:0~3，值越小越高
	bool bReadFromMemery;
	u32 PeriphAddr;
	u32 MemeryAddr;
	bool bCircularMod;
	bool bPeriphIncMod;
	bool bMemoryIncMod;
	u32 PeriphSize;//0=8bit 1=16bit 2=32bit
	u32 MemorySize;
	bool bTransferCompInterrupt;
	u32 NumberOfDataToTransfer;//数据传输量
}DMADef;

void DMA_new(DMA_Channel_TypeDef* CHx,DMADef* pDmaConf)
{
	CHx->CPAR = pDmaConf->PeriphAddr;//设置外设地址，注意PSize
	CHx->CMAR = pDmaConf->MemeryAddr;//存储器地址
	CHx->CNDTR = pDmaConf->NumberOfDataToTransfer;//设置传输数据量
	
	CHx->CCR = 0;//!复位
	
	CHx->CCR = 0//!(DMA_CCRx)
		|pDmaConf->bReadFromMemery<<4//从存储器（外设）读
		|pDmaConf->bCircularMod<<5//循环模式
		|pDmaConf->bPeriphIncMod<<6//外设地址增量模式
		|pDmaConf->bMemoryIncMod<<7//存储器增量模式
		|pDmaConf->PeriphSize<<8//PSIZE:外设数据宽度
		|pDmaConf->MemorySize<<10//MSIZE:存储器数据宽度
		|(3-pDmaConf->Priority)<<12//通道优先级
		|0<<14//非存储器到存储器模式
		|pDmaConf->bTransferCompInterrupt<<1//TCIE.允许传输完成中断
		;//!END
	
	CHx->CCR |= 1<<0;//开启DMA
}

void DMA_Restart(DMA_Channel_TypeDef* CHx,u16 NumberOfDataToTransfer)
{
	CHx->CCR &= ~(1<<0);//关闭DMA传输
	CHx->CNDTR = NumberOfDataToTransfer;
	CHx->CCR |= 1<<0;//开启DMA
}

/* 存储器声明列表 */
extern u16 ADC_DataBuffer[ADC_DMA_BUFFER_LENGTH];

void DMA_init(void)
{
	DMADef DMAConf;
	
	//ADC1 DMA配置
	DMAConf.Priority=1;
	DMAConf.bReadFromMemery=false;
	DMAConf.PeriphAddr=(u32)&ADC1->DR;
	DMAConf.MemeryAddr=(u32)ADC_DataBuffer;
	DMAConf.bPeriphIncMod=false;
	DMAConf.bMemoryIncMod=true;
	DMAConf.PeriphSize=DMA_SIZE_16_BIT;
	DMAConf.MemorySize=DMA_SIZE_16_BIT;
	DMAConf.NumberOfDataToTransfer=ADC_DMA_BUFFER_LENGTH;
	DMAConf.bTransferCompInterrupt=false;
	DMAConf.bCircularMod=true;
	
	DMA_new(DMA1_Channel1,&DMAConf);//配置Channel1，ADC1.DMA
}

/*
void DMA1_Channel1_IRQHandler(void)
{
	DMA1->IFCR |= 1<<1; //清零通道完成中断标志位
}
*/
