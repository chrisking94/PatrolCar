#include "usart.h"
//����

/***˽�к�������***/
u32 calcBRRDiv(u32 BaudRate,u32 Pclk);

//------------------------------------------------------------------
//��������void usart_init(void)
//�����������
//���ز�������
//˵������ʼ�����ڼĴ���
//��ע��DEBUGģʽ�½����ʼ��USART1
//------------------------------------------------------------------
void usart_init(void)
{
    //Usart2 ����
    USART2->BRR = calcBRRDiv(9600,Pclk1_36MHz);//��USART_BRR��
    USART2->GTPR = USART_PSC/2;//��USART_GTPR��
    USART2->CR1 = 0x0000202C;//��USART_CR1)0x000020EC
    USART2->CR2 = 0;				 //��USART_CR2)
    USART2->CR3 = 0;				 //��USART_CR3)
#ifdef DEBUG
//Usart1 ����
    USART1->BRR = calcBRRDiv(9600,Pclk2_72MHz);//��USART_BRR��
    USART1->GTPR = USART_PSC/2;//��USART_GTPR��
    USART1->CR1 = 0x0000202C;//��USART_CR1)
#endif
}

void printChar(char ch,USART_TypeDef* Usart)
{
	while (!(Usart->SR & 1<<7));//!�ȴ��ϴδ������
    Usart->DR = (u16)ch;
}

void printStr(char * str,USART_TypeDef* Usart)
{
    while (*str)
    {
        printChar(*str++,Usart);
    }
}

void printNum(s32 num,USART_TypeDef* Usart)
{
    uchar buf[10],i=0;
    if (num<0)
    {
        num=~num;
        num+=1;
        printChar('-',Usart);
    }
    do
    {
        buf[i++]=(uchar)(num%10);
    } while (num/=10);
    while (i--)
    {
        printChar(buf[i]+0x30,Usart);
    }
}

//------------------------------------------------------------------
//��������void usart_work(void)
//���������null
//���ز�����null
//˵�������ڽ������ݴ���
//------------------------------------------------------------------
bool bProcessCompleted=true;//�ж�һ֡�����Ƿ�����ɣ���һ֡���ݽ�����ϣ���ñ�������Ϊfalse

extern u16 mod;
void usart_work(void)
{
    if (!bProcessCompleted)//���յ�������֡
    {
        
        
        bProcessCompleted=true;
		USART2->CR1 |= 1<<13;//!����Usart2
    }
}

#ifdef DEBUG
void printDebug(char *str,s32 num)
{
	printStr(str,USART1);
	printNum(num,USART1);
	printStr("\r\n",USART1);
}
#endif
//------------------------------------------------------------------
//��������void USART2_IRQHandler(void)
//���������null
//���ز�����null
//˵�������ڽ����жϷ���
//------------------------------------------------------------------
void USART2_IRQHandler(void)
{
    if (USART2->SR & 1<<5)//!�ж϶��Ĵ����Ƿ�ǿ�
    {
		USART2->DR;
		//mod=USART2->DR-48;
		ledp=!ledp;
	}
}

//------------------------------------------------------------------
//��������u32 calcBRRDiv(u32 BaudRate,u32 Pclk)
//���������BaudRate=������,Pclk=RCCʱ��Ƶ��
//���ز�����Ӧ����Ĵ�����ֵ
//˵����������Ӧ�������Լ�ʱ��Ƶ�ʶ�Ӧ��BRRDiv�Ĵ�����Ӧ�������ֵ
//��ע�����㷽�����գ����������ʵĲ�����
//------------------------------------------------------------------
u32 calcBRRDiv(u32 BaudRate,u32 Pclk)
{
    u32 div_mant;
    u32 div_frac;
    float frac;
    div_mant=Pclk/(USART_PSC*BaudRate);
    frac=(float)Pclk/(USART_PSC*BaudRate);
    frac-=div_mant;
    frac*=USART_PSC;
    div_frac=(u32)frac;
    div_frac+=(frac-div_frac)>=0.5?1:0;//��������

    if (div_frac==USART_PSC)//��USART_PSC��λ
    {
        div_frac=0;
        div_mant++;
    }
    div_mant<<=4;
    div_mant|=div_frac;
    return div_mant;
}

