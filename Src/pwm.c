#include "pwm.h"

#define STM32F411xE

#include "stm32f4xx.h"
#include <stdio.h>

/*
 * READ PWM:
 *  • Select the active input for TIMx_CCR1: write the CC1S bits to 01 in the TIMx_CCMR1
	register (TI1 selected).
	• Select the active polarity for TI1FP1 (used both for capture in TIMx_CCR1 and counter
	clear): write the CC1P to ‘0’ and the CC1NP bit to ‘0’ (active on rising edge).
	• Select the active input for TIMx_CCR2: write the CC2S bits to 10 in the TIMx_CCMR1
	register (TI1 selected).
	• Select the active polarity for TI1FP2 (used for capture in TIMx_CCR2): write the CC2P
	bit to ‘1’ and the CC2NP bit to ’0’(active on falling edge).
	• Select the valid trigger input: write the TS bits to 101 in the TIMx_SMCR register
	(TI1FP1 selected).
	• Configure the slave mode controller in reset mode: write the SMS bits to 100 in the
	TIMx_SMCR register.
	• Enable the captures: write the CC1E and CC2E bits to ‘1 in the TIMx_CCER register.
 * */

#define GPIOAEN				(1U<<0)
#define GPIOCEN				(1U<<2)

#define TIM2EN				(1U << 0)

#define TIM3EN				(1U << 1)

#define CC2P				(1U << 5)
#define CC1E				(1U << 0)
#define CC2E				(1U << 4)

#define TIM5EN				(1U << 3)

#define CEN					(1U << 0)
#define UG 					(1U << 0)

#define AF2 				(1U << 1)
#define AF1					(0x1U)

#define	ARPE				(1U << 7)

#define OC2PE				(1U << 11)
#define OC1PE				(1U << 3)

#define OC4PE				(1U << 11)
#define OC3PE				(1U << 3)

static volatile uint32_t high_ticks = 0;
static volatile uint32_t period_ticks = 0;
uint32_t count = 0;
uint32_t period;

void tim2_pwm_in_init_2(void)
{
	RCC->AHB1ENR |= GPIOAEN;
	RCC->APB1ENR |= TIM2EN;

	TIM2->PSC = 159;

	TIM2->ARR = 0xFFFFFFFF;

	//CC1S = 01, IC1 input = TI1
	TIM2->CCMR1 |= 1U<<0;//
	TIM2->CCMR1 &= ~(1U<<1);//

	//T1FP1 00 - value capture on Rising Edge
	TIM2->CCER |= (1U << 1 | 1U << 3);

	//PA0 mode = AF. AFRL0 = AF1;
	GPIOA->MODER |= 1U << 1;
	GPIOA->MODER &= ~(1U << 0);

	GPIOA->AFR[0] &= ~(0xFU << 0);
	GPIOA->AFR[0] |= 1U << 0;

	//Turn on CC1E
	TIM2->CCER |= CC1E;

	TIM2->CR1 |= CEN;
}

void tim2_pwm_in_init(void)
{
	RCC->AHB1ENR |= GPIOAEN;
	RCC->APB1ENR |= TIM2EN;

	TIM2->PSC = 159;

	TIM2->ARR = 0xFFFFFFFF;

	//CC1S = 01, IC1 input = TI1
	TIM2->CCMR1 |= 1U<<0;//
	TIM2->CCMR1 &= ~(1U<<1);//

	//CC2S = 10, IC2 input = TI1
	TIM2->CCMR1 &= ~(1U << 8);
	TIM2->CCMR1 |= 1U << 9;

	//T1FP1 00 - value capture on Rising Edge
	TIM2->CCER &= ~(1U << 1 | 1U << 3);

	//T1FP1 01 - value capture on Falling Edge
	TIM2->CCER |= 1U<<5;
	TIM2->CCER &= ~(1U<<7);

	//SMCR - input trigger on T1FP1
	TIM2->SMCR &= ~(1U << 4 | 1U << 5 | 1U << 6);
	TIM2->SMCR |= 1U<<4 | 1U<<6;
	TIM2->SMCR &= ~(1U<<5);

	TIM2->SMCR &= ~(1U << 2 | 1U << 1 | 1U << 0);
	TIM2->SMCR |= 1U<<2;

	//PA0 mode = AF. AFRL0 = AF1;
	GPIOA->MODER |= 1U << 1;
	GPIOA->MODER &= ~(1U << 0);

	GPIOA->AFR[0] &= ~(0xFU << 0);
	GPIOA->AFR[0] |= 1U << 0;

	//Turn on CC1E CC2E
	TIM2->CCER |= CC1E;
	TIM2->CCER |= CC2E;

	TIM2->CR1 |= CEN;
}

uint32_t high_tick_read(void)
{
	count = TIM2->CNT;
	while(!(TIM2->SR & (1U<<2))) {
		if(count + 4000 < TIM2->CNT)
		{
			return high_ticks = 4000;
		}
	}

	high_ticks = TIM2->CCR2;
	TIM2->SR &= ~(1U<<2);

	return high_ticks;
}

uint32_t period_tick_read(void)
{
	count = TIM2->CNT;
	while(!(TIM2->SR & (1U<<1)))
	{
		if(count + 4000 < TIM2->CNT)
		{
			return period_ticks = 4000;
		}
	}


	period_ticks = TIM2->CCR1;
	TIM2->SR &= ~(1U<<1);

	return period_ticks;
}

float pwm_get_frequency(void)
{
	period_tick_read();
    const float tim_clk = 1000000.0f;
    return tim_clk/(float)period_ticks;
}

void tim2_toggle_pwm_out_init(uint32_t period_t, uint32_t initial_duty)
{
	period = period_t;
	RCC->APB1ENR |= TIM2EN;//enable tim2
	RCC->AHB1ENR |= GPIOAEN;//enable GPIOA

	GPIOA->MODER |= (1U << 11);// A5
	GPIOA->MODER &= ~(1U << 10);//A5

	GPIOA->AFR[0] &= ~(0xFU << 4 * 5);
	GPIOA->AFR[0] |= (AF1 << 4 * 5);//AF1

    TIM2->CCMR1 &= ~(7U << 4);
    TIM2->CCMR1 |= 3U << 4;

	TIM2->CR1 |= ARPE;
	TIM2->CCER |= CC1E;

	if(TIM2->PSC != 15)
	{
		TIM2->PSC = 15;//15+1 -- 1MHZ
		TIM2->EGR |= UG;//set UG
	}

    TIM2->DIER |= TIM_DIER_CC1IE;

    NVIC_EnableIRQ(TIM2_IRQn);

	TIM2->CCR1 = TIM2->CNT + initial_duty;//0 -- default

	TIM2->CR1 |= CEN;
}

void tim2_pwm_out_init(uint32_t period, uint32_t initial_duty)
{
	RCC->APB1ENR |= TIM2EN;//enable tim2
	RCC->AHB1ENR |= GPIOAEN;//enable GPIOA

	GPIOA->MODER |= (1U << 11);// A5
	GPIOA->MODER &= ~(1U << 10);//A5

	GPIOA->AFR[0] &= ~(0xFU << 4 * 5);
	GPIOA->AFR[0] |= (AF1 << 4 * 5);//AF1

	TIM2->PSC = 15;//15+1 -- 1MHZ
	TIM2->ARR = period - 1;//4000 -- default
	TIM2->CCR2 = initial_duty;//0 -- default
	TIM2->CCMR1 |= (1U << 6 | 1U << 5);//
	TIM2->CCMR1 &= ~(1U << 4);
	TIM2->CCMR1 |= OC1PE;
	TIM2->CR1 |= ARPE;
	TIM2->CCER |= CC1E;

	TIM2->EGR |= UG;//set UG

	TIM2->CR1 |= CEN;
}


void tim3_gpioc_4channel_pwm_out_init(uint32_t period, uint32_t initial_duty)
{
	RCC->APB1ENR |= TIM3EN;//enable tim2
	RCC->AHB1ENR |= GPIOCEN;//enable GPIOA
	//PC6-9 MODER 10; AF2
	GPIOC->MODER &= ~(3U << 2 * 6);//C6
	GPIOC->MODER |= (2U << 2 * 6);// C6

	GPIOC->MODER &= ~(3U << 2 * 7);//C7
	GPIOC->MODER |= (2U << 2 * 7);// C7

	GPIOC->MODER &= ~(3U << 2 * 8);//C8
	GPIOC->MODER |= (2U << 2 * 8);// C8

	GPIOC->MODER &= ~(3U << 2 * 9);//C9
	GPIOC->MODER |= (2U << 2 * 9);// C9

	GPIOC->AFR[0] &= ~(0xFU << 4 * 6);//C6
	GPIOC->AFR[0] |= (AF2 << 4 * 6);//AF2

	GPIOC->AFR[0] &= ~(0xFU << 4 * 7);//C7
	GPIOC->AFR[0] |= (AF2 << 4 * 7);//AF2

	GPIOC->AFR[1] &= ~(0xFU << 4 * (8 - 8));//C8
	GPIOC->AFR[1] |= (AF2 << 4 * (8 - 8));//AF2

	GPIOC->AFR[1] &= ~(0xFU << 4 * (9 - 8));//C9
	GPIOC->AFR[1] |= (AF2 << 4 * (9 - 8));//AF2

	TIM3->PSC = 15;//15+1 -- 1MHZ
	TIM3->ARR = period - 1;//4000 -- default

	TIM3->CCR1 = initial_duty;//0 -- default
	TIM3->CCR2 = initial_duty;//0 -- default
	TIM3->CCR3 = initial_duty;//0 -- default
	TIM3->CCR4 = initial_duty;//0 -- default

	TIM3->CCMR1 &= ~(7U << 4);
	TIM3->CCMR1 |= (6U << 4);//
	TIM3->CCMR1 |= OC1PE;

	TIM3->CCMR1 &= ~(7U << (4 + 8));
	TIM3->CCMR1 |= (6U << (4 + 8));//
	TIM3->CCMR1 |= OC2PE;

	TIM3->CCMR2 &= ~(7U << 4);
	TIM3->CCMR2 |= (6U << 4);//
	TIM3->CCMR2 |= OC3PE;

	TIM3->CCMR2 &= ~(7U << (4 + 8));
	TIM3->CCMR2 |= (6U << (4 + 8));//
	TIM3->CCMR2 |= OC4PE;

	TIM3->CR1 |= ARPE;

	TIM3->CCER |= CC1E;
	TIM3->CCER |= CC1E << 4;//CC2E
	TIM3->CCER |= CC1E << 8;//CC3E
	TIM3->CCER |= CC1E << 12;//CC4E

	TIM3->EGR |= UG;//set UG

	TIM3->CR1 |= CEN;
}

void tim3_gpioc6_pwm_duty_set(uint32_t duty, uint32_t duty_min, uint32_t duty_max, uint32_t min, uint32_t max)
{
	uint32_t duty_value = duty - duty_min;
	uint32_t duty_adjusted = min + (duty_value * (max-min))/duty_max;
	TIM3->CCR1 = ((TIM3->ARR + 1) * duty_adjusted)/(TIM3->ARR + 1);
}

void tim3_gpioc7_pwm_duty_set(uint32_t duty, uint32_t duty_min, uint32_t duty_max, uint32_t min, uint32_t max)
{
	uint32_t duty_value = duty - duty_min;
	uint32_t duty_adjusted = min + (duty_value * (max-min))/duty_max;
	TIM3->CCR2 = ((TIM3->ARR + 1) * duty_adjusted)/(TIM3->ARR + 1);
}

void tim3_gpioc8_pwm_duty_set(uint32_t duty, uint32_t duty_min, uint32_t duty_max, uint32_t min, uint32_t max)
{
	uint32_t duty_value = duty - duty_min;
	uint32_t duty_adjusted = min + (duty_value * (max-min))/duty_max;
	TIM3->CCR3 = ((TIM3->ARR + 1) * duty_adjusted)/(TIM3->ARR + 1);
}

void tim3_gpioc9_pwm_duty_set(uint32_t duty, uint32_t duty_min, uint32_t duty_max, uint32_t min, uint32_t max)
{
	uint32_t duty_value = duty - duty_min;
	uint32_t duty_adjusted = min + (duty_value * (max-min))/duty_max;
	TIM3->CCR4 = ((TIM3->ARR + 1) * duty_adjusted)/(TIM3->ARR + 1);
}

void tim3_pwm_out_init(uint32_t period, uint32_t initial_duty)
{
	RCC->APB1ENR |= TIM3EN;//enable tim2
	RCC->AHB1ENR |= GPIOAEN;//enable GPIOA

	GPIOA->MODER |= (1U << 13);// A6
	GPIOA->MODER &= ~(1U << 12);//A6

	GPIOA->AFR[0] &= ~(0xFU << 4 * 6);
	GPIOA->AFR[0] |= (AF2 << 4 * 6);//AF1

	TIM3->PSC = 15;//15+1 -- 1MHZ
	TIM3->ARR = period - 1;//4000 -- default
	TIM3->CCR2 = initial_duty;//0 -- default
	TIM3->CCMR1 |= (1U << 6 | 1U << 5);//
	TIM3->CCMR1 &= ~(1U << 4);
	TIM3->CCMR1 |= OC1PE;
	TIM3->CR1 |= ARPE;
	TIM3->CCER |= CC1E;

	TIM3->EGR |= UG;//set UG

	TIM3->CR1 |= CEN;
}

void tim3_pwm_duty_set(uint32_t duty, uint32_t duty_min, uint32_t duty_max, uint32_t min, uint32_t max)
{
	uint32_t duty_value = duty - duty_min;
	uint32_t duty_adjusted = min + (duty_value * (max-min))/duty_max;
	TIM3->CCR1 = ((TIM3->ARR + 1) * duty_adjusted)/(TIM3->ARR + 1);
}

void tim5_pwm_out_init(uint32_t period, uint32_t initial_duty)
{
	RCC->APB1ENR |= TIM5EN;//enable tim2
	RCC->AHB1ENR |= GPIOAEN;//enable GPIOA

	GPIOA->MODER |= (1U << 3);// A1
	GPIOA->MODER &= ~(1U << 2);//A1

	GPIOA->AFR[0] &= ~(0xFU << 4 * 1);
	GPIOA->AFR[0] |= (AF2 << 4 * 1);//AF1

	TIM5->PSC = 15;//15+1 -- 1MHZ
	TIM5->ARR = period - 1;//4000 -- default
	TIM5->CCR2 = initial_duty;//0 -- default
	TIM5->CCMR1 |= (1U << 6 | 1U << 5) << 8;//
	TIM5->CCMR1 &= ~((1U << 4) << 8);
	TIM5->CCMR1 |= OC2PE;
	TIM5->CR1 |= ARPE;
	TIM5->CCER |= CC2E;

	TIM5->EGR |= UG;//set UG

	TIM5->CR1 |= CEN;
}

void tim5_pwm_duty_set(uint32_t duty, uint32_t duty_min, uint32_t duty_max, uint32_t min, uint32_t max)
{
	uint32_t duty_value = duty - duty_min;
	uint32_t duty_adjusted = min + (duty_value * (max-min))/duty_max;
	TIM5->CCR2 = ((TIM5->ARR+1) * duty_adjusted)/(TIM5->ARR+1);
}

void tim2_pwm_duty_set(uint32_t duty, uint32_t duty_min, uint32_t duty_max, uint32_t min, uint32_t max)
{
	uint32_t duty_value = duty - duty_min;
	uint32_t duty_adjusted = min + (duty_value * (max-min))/duty_max;
	TIM2->CCR1 = TIM2->CNT + ((period) * duty_adjusted)/(period);
}

uint32_t tim5_cnt_val(void)
{
	while(!(TIM5->SR & 1U<<2))
	{
		high_ticks = high_ticks;
	}

	TIM5->SR &= ~(1U<<2);
	return TIM5->CNT;
}
/*
 * Use TIM5 for PWM signal generation
 * use PA1 for TIM5_CH2 as output
 * set ARR for frequency
 * set CCRx for active duty
 * CR1.ARPE |= 1 for buffering
 * CCMR1.OC2M(12-14) = 110, chooses active level;
 * CCMR1.OC2PE(bit 11) = 1
 * TIM5_EGR = UG
 * TIM5.CCER = CC2P | CC2E
 * TIM5.PSC for frequency
 * CC2IF check for compare match
 * */
