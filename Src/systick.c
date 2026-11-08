#include "systick.h"

#define STM32F411xE

#include "stm32f4xx.h"

#define SYSTICK_LOAD_VAL		16000
#define CTRL_CLKSRC				(1U<<2)
#define CTRL_ENABLE 			(1U<<0)

#define CTRL_TICKINT 			(1U<<1)

#define CTRL_COUNTFLAG				(1U<<16)

void systick_1hz_interrupt(void)
{
	SysTick->LOAD |= 16000000;
	SysTick->VAL = 0;

	SysTick->CTRL |= CTRL_TICKINT;

	SysTick->CTRL |= CTRL_ENABLE;
	SysTick->CTRL |= CTRL_CLKSRC;
}

void systickDelayMs(int n)
{
	SysTick->LOAD = 16000;
	SysTick->VAL = 0;

	SysTick->CTRL |= CTRL_ENABLE;
	SysTick->CTRL |= CTRL_CLKSRC;

	for(int i=0; i < n; i++)
	{
		while(!(SysTick->CTRL & CTRL_COUNTFLAG)){}
	}

	SysTick->CTRL = 0;
}

void systickDelayUs(int n)
{
	SysTick->LOAD = 16;
	SysTick->VAL = 0;

	SysTick->CTRL |= CTRL_ENABLE;
	SysTick->CTRL |= CTRL_CLKSRC;

	for(int i=0; i < n; i++)
	{
		while(!(SysTick->CTRL & CTRL_COUNTFLAG)){}
	}

	SysTick->CTRL = 0;
}

uint32_t read_calib()
{
	return SysTick->CALIB;
}
