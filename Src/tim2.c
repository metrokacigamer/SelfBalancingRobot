#define STM32F411xE

#include "stm32f4xx.h"
#include "tim2.h"

#define TIM2EN				(1U<<0)
#define CEN					(1U<<0)

#define TIM2PSC				16
#define TIM2ARR				10000

#define SR_UIF				(1U<<0)

void tim2_init(void)
{
	RCC->APB1ENR |= TIM2EN;

	TIM2->PSC = TIM2PSC - 1;

	TIM2->CNT = 0;

	TIM2->EGR |= 1U << 0;//set UG
	TIM2->CR1 |= CEN;
}

uint32_t tim2_count(void)
{
	return TIM2->CNT;
}


void tim2_1hz_init(void)
{
	RCC->APB1ENR |= TIM2EN;

	TIM2->PSC = TIM2PSC - 1;

	TIM2->ARR = TIM2ARR - 1;

	TIM2->CNT = 0;

	TIM2->CR1 |= CEN;
}

uint32_t tim2_cycle_complete(void)
{
	return TIM2-> SR & SR_UIF;
}

void tim2_clear_UIF(void)
{
	TIM2->SR &= ~SR_UIF;
}

