#ifndef SENSER_H_
#define SENSER_H_

#include "adc.h"

#define SENSER_MAXVAL 512

#define SENSER_ORIGIN_MIN 700//δ������Ĵ�����ֵ���ֵ

void Senser_getsv(u8 sen);//0:��1:��
void Senser_reset(void);

#endif
