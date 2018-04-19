#ifndef COMMON_H
#define COMMON_H

#include "stm32f10x.h"
#include "system.h"

#define DEBUG
//#define IR_CTRL_
#define USART_ON

#define uchar unsigned char
#define u8 unsigned char
#define s8 char
#define uint u32
#define int s32
#define bit u8
#define bool u32
#define true (bool)1
#define false (bool)0

#define ledp PAout(15)
#define obeep PAout(14)

void common_init(void);
void delay(u32 s);
void delayms(u32 ms);
void delayus(u16 us);
void waitWhile(bool condition);
void beep(u16 time ,u16 interval);
void startTiming(void);
u32 readTimingms(void);

#endif
