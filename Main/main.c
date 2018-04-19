#include "adc.h"
#include "common.h"
#include "usart.h"
#include "L298N.h"
#include "senser.h"
#include "NVIC.h"
#include "ir1838.h"
#include "DMA.h"

#define kP 11
#define kdI 200 //nI除数 kI=1/kdI
#define kD 120
#define kdPID 28//PID除数
#define D_INTERVAL 1200
#define I_INTERVAL 15

#define ONLINE_MINVAL 25
#define POSVAL_MAX SENSER_MAXVAL*2//位置量值域(-POSVAL_MAX,POSVAL_MAX)

#define SPEED_MIN 1700
#define SPEED_NORMAL 1950
#define SPEED_MAX 2000

#define CHECK_WAIT_TIMEOUT 100000

extern u16 ADC_DataBuffer[ADC_DMA_BUFFER_LENGTH];
extern u32 senVals[3];

u16 mod=0;

int main()
{
	s32 PID;

	s32 nP=0;
	s32 nI=0;
	s32 nD=0;

	s32 nLastP=0;
	
	u32 nDC=D_INTERVAL;
	u32 nIC=I_INTERVAL;
	
	s32 carSpeed=SPEED_NORMAL;
	
	u32 checkWait=0;
	u8 adjustMod=100;

	common_init();
	ADC_init();
	#ifdef USART_ON
		usart_init();
	#endif
	L298N_init();
	#ifdef IR_CTRL_
		IR1838_init();
	#else
		//mod=1;
	#endif
	DMA_init();
	NVIC_init();
	
	delayms(120);//等待DMA将 adc_buffer 填满，避免senser校准值出错（如果有0会导致minvs不正确）
	
	while (1)
	{
		#ifdef IR_CTRL_
			Ircordpro();
		#endif

		switch (mod)
		{
		case 0:
			//等待车被放上跑道
			senVals[0]=ADC_DMA_getVal(0);
			senVals[1]=ADC_DMA_getVal(1);
			senVals[2]=ADC_DMA_getVal(2);
			#ifdef USART_ON
				#define LSHIFTBIT	9
				#define	SENVALGATE	9
			//printDebug("senVals[0]=",senVals[0]);
			//printDebug("senVals[1]=",senVals[1]);
			//printDebug("senVals[2]=",senVals[2]);
			//printDebug("-------------\n",0);
			if(senVals[2]>SENVALGATE)
			{
				senVals[2]-=SENVALGATE;
			}
			else
			{
				senVals[2]=0;
			}
			senVals[2]<<=LSHIFTBIT;
			
			L298N_setLS(senVals[2]);
			L298N_setRS(senVals[2]);
			//delayms(3000);
			continue;
			#endif
			if (senVals[0]>SENSER_ORIGIN_MIN&&senVals[0]>SENSER_ORIGIN_MIN&&senVals[0]>SENSER_ORIGIN_MIN)//车被提起
			{
				checkWait=CHECK_WAIT_TIMEOUT;
				adjustMod=100;
				L298N_setLS(0);
				L298N_setRS(0);
			}
			else
			{
				if(adjustMod==100)
				{
					adjustMod=0;
					Senser_reset();
				}
			}
			
			switch (adjustMod)
			{
			case 0:
				if (checkWait)
				{
					checkWait--;
				}
				else
				{
					adjustMod=1;
					beep(900,300);
				}
				break;
			case 1:
				//将左传感器移动到线上
				L298N_setLS(1400);
				L298N_setRS(0);
				if(senVals[0]>SENSER_ORIGIN_MIN)
				{
					adjustMod=2;
				}
				break;
			case 2:
				//将左传感器移动到线右边
				if(senVals[0]<SENSER_ORIGIN_MIN)
				{
					L298N_setLS(0);
					L298N_setRS(0);
					adjustMod=3;
					beep(900,300);
				}
				break;
			case 3:
				//左转开始扫描，直到右传感在线上
				L298N_setLS(0);
				L298N_setRS(1400);
				if(senVals[2]>SENSER_ORIGIN_MIN)
				{
					adjustMod=4;
				}
				break;
			case 4:
				//直到右传感器移动到线左边
				if(senVals[2]<SENSER_ORIGIN_MIN)
				{
					adjustMod=5;
					beep(900,300);
				}
				break;
			case 5:
				//右转直到中间传感器在线上
				L298N_setLS(1400);
				L298N_setLS(0);
				if(senVals[1]>SENSER_ORIGIN_MIN)
				{
					beep(800,300);
					delayms(10);
					beep(600,300);
					delayms(10);
					beep(600,300);
					delayms(10);
					beep(600,300);
					mod=1;
				}
			}
			
			if(adjustMod==3||adjustMod==4)
			{
				//校正
				Senser_getsv(0);
				Senser_getsv(1);
				Senser_getsv(2);
			}
		case 1:
			Senser_getsv(0);
			Senser_getsv(1);
			Senser_getsv(2);
			
			/* ------------P------------ */
			/* 计算位置 */
			if (senVals[1]>ONLINE_MINVAL)
			{
				//线在中间传感器下
				if (carSpeed<SPEED_MAX)
				{
					//加速
					carSpeed++;
				}

				if (senVals[0]>ONLINE_MINVAL)
				{
					//中间偏左
					nP=POSVAL_MAX/2-senVals[1];
				}
				else
				{
					//中间偏右
					nP=senVals[1]-POSVAL_MAX/2;
				}
			}
			else if (senVals[0]>ONLINE_MINVAL)
			{
				//线在左边传感器偏左
				nP=POSVAL_MAX-senVals[0];
			}
			else if (senVals[2]>ONLINE_MINVAL)
			{
				//线在右边传感器偏右
				nP=senVals[2]-POSVAL_MAX;
			}
			else
			{
				//线离开传感器检测范围
				if (carSpeed>SPEED_MIN)
				{
					//减速
					carSpeed--;
				}
				if (nP<0)
				{
					nP=-POSVAL_MAX;
				}
				else
				{
					nP=POSVAL_MAX;
				}
			}
			//nP/=4;
			
			/* ------------I------------ */
			if(nIC)
			{
				nIC--;
			}
			else
			{
				nIC=I_INTERVAL;
				
				if(nP>ONLINE_MINVAL||nP<-ONLINE_MINVAL)
				{
					nI+=nP/20;//nI=9*nI/10;
				}
				else
				{
					nI=0;
				}
			}
			
			/* ------------D------------ */
			if(nDC)
			{
				nDC--;
			}
			else
			{
				nDC=D_INTERVAL;
				
				nD=nP-nLastP;
				nLastP=nP;
			}
			
			/* PID */
			PID=nP*kP + nI/kdI + nD*kD;
			PID/=kdPID;
			
			L298N_setSpeed(carSpeed,PID);
			
			/*
			printDebug("nP=",nP*kP);
			printDebug("nI=",nI/kdI);
			printDebug("nD=",nD*kD);
			printDebug("PID=",PID);
			printChar('\n',USART2);
			
			printDebug("LS=",TIM4->CCR3);
			printDebug("RS=",TIM4->CCR4);
			printChar('\n',USART2);
			
			printDebug("senVals[0]=",senVals[0]);
			printDebug("senVals[1]=",senVals[1]);
			printDebug("senVals[2]=",senVals[2]);
			printChar('\n',USART2);
			//delayms(1000);
			// */

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
