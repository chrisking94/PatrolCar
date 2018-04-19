/*********************************************
USART2{TX=PA2,Rx=PA3}，USART1{TX=PA9，Rx=PA10}
*********************************************/
#ifndef USART_H_
#define USART_H_

#include "Common.h"

#define Pclk1_36MHz 36000000 //其他USART
#define Pclk2_72MHz 72000000 //USART1
#define USART_PSC 16				 //Prescaler Value 时钟预分频

//#define CrLf {0x0D,0x0A,0x00}

void usart_init(void);
void usart_work(void);

void printChar(char ch,USART_TypeDef* Usart);
void printStr(char * str,USART_TypeDef* Usart);
void printNum(s32 num,USART_TypeDef* Usart);

#ifdef DEBUG
void printDebug(char *str,s32 num);
#endif

#endif
