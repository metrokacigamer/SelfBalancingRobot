#define STM32F411xE

#include "stm32f4xx.h"
#include "tim3.h"

#define TIM3EN				(1U<<1)
#define CEN					(1U<<0)

#define TIM3PSC				16

#define SR_UIF				(1U<<0)


void tim3_init(void)
{
	RCC->APB1ENR |= TIM3EN;

	TIM3->PSC = TIM3PSC - 1;

	TIM3->CNT = 0;

	TIM3->EGR |= 1U << 0;//set UG
	TIM3->CR1 |= CEN;
}

uint32_t tim3_count(void)
{
	return TIM3->CNT;
}
