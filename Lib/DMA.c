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
	u32 Priority;//Priority:0~3��ֵԽСԽ��
	bool bReadFromMemery;
	u32 PeriphAddr;
	u32 MemeryAddr;
	bool bCircularMod;
	bool bPeriphIncMod;
	bool bMemoryIncMod;
	u32 PeriphSize;//0=8bit 1=16bit 2=32bit
	u32 MemorySize;
	bool bTransferCompInterrupt;
	u32 NumberOfDataToTransfer;//���ݴ�����
}DMADef;

void DMA_new(DMA_Channel_TypeDef* CHx,DMADef* pDmaConf)
{
	CHx->CPAR = pDmaConf->PeriphAddr;//���������ַ��ע��PSize
	CHx->CMAR = pDmaConf->MemeryAddr;//�洢����ַ
	CHx->CNDTR = pDmaConf->NumberOfDataToTransfer;//���ô���������
	
	CHx->CCR = 0;//!��λ
	
	CHx->CCR = 0//!(DMA_CCRx)
		|pDmaConf->bReadFromMemery<<4//�Ӵ洢�������裩��
		|pDmaConf->bCircularMod<<5//ѭ��ģʽ
		|pDmaConf->bPeriphIncMod<<6//�����ַ����ģʽ
		|pDmaConf->bMemoryIncMod<<7//�洢������ģʽ
		|pDmaConf->PeriphSize<<8//PSIZE:�������ݿ��
		|pDmaConf->MemorySize<<10//MSIZE:�洢�����ݿ��
		|(3-pDmaConf->Priority)<<12//ͨ�����ȼ�
		|0<<14//�Ǵ洢�����洢��ģʽ
		|pDmaConf->bTransferCompInterrupt<<1//TCIE.����������ж�
		;//!END
	
	CHx->CCR |= 1<<0;//����DMA
}

void DMA_Restart(DMA_Channel_TypeDef* CHx,u16 NumberOfDataToTransfer)
{
	CHx->CCR &= ~(1<<0);//�ر�DMA����
	CHx->CNDTR = NumberOfDataToTransfer;
	CHx->CCR |= 1<<0;//����DMA
}

/* �洢�������б� */
extern u16 ADC_DataBuffer[ADC_DMA_BUFFER_LENGTH];

void DMA_init(void)
{
	DMADef DMAConf;
	
	//ADC1 DMA����
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
	
	DMA_new(DMA1_Channel1,&DMAConf);//����Channel1��ADC1.DMA
}

/*
void DMA1_Channel1_IRQHandler(void)
{
	DMA1->IFCR |= 1<<1; //����ͨ������жϱ�־λ
}
*/
