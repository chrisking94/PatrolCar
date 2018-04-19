#include "adc.h"
#include "common.h"
#include "Patrol.h"
#include "usart.h"
#include "L298N.h"
#include "senser.h"
#include "NVIC.h"
#include "ir1838.h"

#define kP 8
#define kdI 1 //nI除数 kI=1/kdI
#define kD 30

#define PID_MAX 5000
#define nI_MAX 30000
u16 mod=1;
int main()
{
	u16 senVals[2];
	s32 PID;

	s32 nP=0;
	s32 nI=0;
	s32 nD=0;

	s32 nLastP=0;

	s32 carSpeed=6000;

	common_init();
	ADC_init();
	usart_init();
	L298N_init();
	IR1838_init();
	NVIC_init();
	

	while (1)
	{
		Ircordpro();
		senVals[0]=Senser_getsv(0);
		senVals[1]=Senser_getsv(1);
		switch (mod)
		{
		case 0:
			//校正
			L298N_setLS(0);
			L298N_setRS(0);
			break;
		case 1:
			if (senVals[0]>30||senVals[1]>30)//在跑道上
			{
				//未偏离时采用传感器即时值
				nP = senVals[0]-senVals[1];//左-右，黑线上值大于白纸
			}
			else
			{ 
				if(nP<0)
				{
					nP=-1000;
				}
				else
				{
					nP=1000;
				}
				//nP=nLastP;
			}
			nP/=4;
			
			nI=3*nI/4+nP;
			
			nD=2*nP-nLastP;
			nLastP=nP;
			
			PID=nP*kP + nI/kdI + nD*kD;
			
			
			/*
			printDebug("nP=",nP*kP);
			printDebug("nI=",nI/kdI);
			printDebug("nD=",nD*kD);
			printDebug("PID=",PID);
			printChar('\n',USART2);
			delayms(100);
			*/
			
			PID/=3;

			L298N_setLS(carSpeed-PID);
			L298N_setRS(carSpeed+PID);
			
			break;
		case 3:
			L298N_setLS(0);
			L298N_setRS(0);
			break;
		}
#ifdef DEBUG
		/*
				printDebug("lsen=",senVals[0]);
				printDebug("rsen=",senVals[1]);
				printDebug("pid=",PID);
				printChar('\n',USART2);
				delayms(1000);
		*/

		/*
				L298N_setLS(800);
				L298N_setRS(800);
				ledp=!ledp;
				delayms(2000);
		*/
#endif
	}

	//return 0;
}
