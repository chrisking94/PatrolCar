#ifndef TACHOMETER_H_
#define TACHOMETER_H_

#include "Common.h"

#define SpeedMeasureCycle 200//�������ڵ�λms

extern u16 TMLSpeed;
extern u16 TMRSpeed;

void Tachometer_init(void);

#endif
