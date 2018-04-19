#include "usart.h"
//串口

/***私有函数声明***/
u32 calcBRRDiv(u32 BaudRate,u32 Pclk);

//------------------------------------------------------------------
//函数名：void usart_init(void)
//输入参数：无
//返回参数：无
//说明：初始化串口寄存器
//备注：DEBUG模式下将会初始化USART1
//------------------------------------------------------------------
void usart_init(void)
{
    //Usart2 配置
    USART2->BRR = calcBRRDiv(9600,Pclk1_36MHz);//（USART_BRR）
    USART2->GTPR = USART_PSC/2;//（USART_GTPR）
    USART2->CR1 = 0x0000202C;//（USART_CR1)0x000020EC
    USART2->CR2 = 0;				 //（USART_CR2)
    USART2->CR3 = 0;				 //（USART_CR3)
#ifdef DEBUG
//Usart1 配置
    USART1->BRR = calcBRRDiv(9600,Pclk2_72MHz);//（USART_BRR）
    USART1->GTPR = USART_PSC/2;//（USART_GTPR）
    USART1->CR1 = 0x0000202C;//（USART_CR1)
#endif
}

void printChar(char ch,USART_TypeDef* Usart)
{
	while (!(Usart->SR & 1<<7));//!等待上次传输结束
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
//函数名：void usart_work(void)
//输入参数：null
//返回参数：null
//说明：串口接收数据处理
//------------------------------------------------------------------
bool bProcessCompleted=true;//判断一帧数据是否处理完成，当一帧数据接收完毕，则该变量被置为false

extern u16 mod;
void usart_work(void)
{
    if (!bProcessCompleted)//接收到新数据帧
    {
        
        
        bProcessCompleted=true;
		USART2->CR1 |= 1<<13;//!启动Usart2
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
//函数名：void USART2_IRQHandler(void)
//输入参数：null
//返回参数：null
//说明：串口接收中断服务
//------------------------------------------------------------------
void USART2_IRQHandler(void)
{
    if (USART2->SR & 1<<5)//!判断读寄存器是否非空
    {
		USART2->DR;
		//mod=USART2->DR-48;
		ledp=!ledp;
	}
}

//------------------------------------------------------------------
//函数名：u32 calcBRRDiv(u32 BaudRate,u32 Pclk)
//输入参数：BaudRate=波特率,Pclk=RCC时钟频率
//返回参数：应置入寄存器的值
//说明：计算相应波特率以及时钟频率对应的BRRDiv寄存器中应该填入的值
//备注：计算方法参照（分数波特率的产生）
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
    div_frac+=(frac-div_frac)>=0.5?1:0;//四舍五入

    if (div_frac==USART_PSC)//满USART_PSC进位
    {
        div_frac=0;
        div_mant++;
    }
    div_mant<<=4;
    div_mant|=div_frac;
    return div_mant;
}

