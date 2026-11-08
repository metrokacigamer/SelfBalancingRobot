#include "stepper_motor.h"

#define STM32F411xE

#include "stm32f4xx.h"

#define GPIOCEN				(1U << 2)
#define GPIOAEN				(1U << 0)

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


#define DIR_PIN_1 			1U
#define STEP_PIN_1 			0U

#define DIR_PIN_2 			2U
#define STEP_PIN_2 			3U

#define EN_PIN_2 			7U
#define EN_PIN_1 			6U

#define FORWARD 			1U
#define REVERSE 			0U

#define ACCEL				110U
#define DECEL 				105U

#define DISTANCE 500000UL

int16_t current_speed, target_speed;

void disable_driver(void)
{
	GPIOA->BSRR = (1U << EN_PIN_2);
	GPIOA->BSRR = (1U << EN_PIN_1);
}

void enable_driver(void)
{
	GPIOA->BSRR = (1U << (16 + EN_PIN_2));
	GPIOA->BSRR = (1U << (16 + EN_PIN_1));
}

void motor_init(uint16_t period)
{
	//DIR pin enable
	RCC->AHB1ENR |= GPIOAEN;

	disable_driver();

	GPIOA->MODER &= ~(3U << 2 * EN_PIN_1);
	GPIOA->MODER |= (1U << 2 * EN_PIN_1);

	GPIOA->MODER &= ~(3U << 2 * EN_PIN_2);
	GPIOA->MODER |= (1U << 2 * EN_PIN_2);

	GPIOA->MODER &= ~(3U << 2 * DIR_PIN_1);
	GPIOA->MODER |= (1U << 2 * DIR_PIN_1);

	GPIOA->MODER &= ~(3U << 2 * DIR_PIN_2);
	GPIOA->MODER |= (1U << 2 * DIR_PIN_2);

	//STEP pin enable
	RCC->APB1ENR |= TIM2EN;

	GPIOA->MODER &= ~(3U << 2 * STEP_PIN_1);//A0
	GPIOA->MODER |= (2U << 2 * STEP_PIN_1);// A0

	GPIOA->MODER &= ~(3U << 2 * STEP_PIN_2);//A3
	GPIOA->MODER |= (2U << 2 * STEP_PIN_2);// A3

	GPIOA->AFR[0] &= ~(0xFU << 4 * STEP_PIN_1);//A0
	GPIOA->AFR[0] |= (AF1 << 4 * STEP_PIN_1);//AF1

	GPIOA->AFR[0] &= ~(0xFU << 4 * STEP_PIN_2);//A3
	GPIOA->AFR[0] |= (AF1 << 4 * STEP_PIN_2);//AF1


	TIM2->PSC = 15;//15+1 -- 1MHZ
	TIM2->ARR = period - 1;
	TIM2->CCR1 = 5;
	TIM2->CCR4 = 5;

	TIM2->CCMR1 &= ~(7U << 4);
	TIM2->CCMR1 |= (6U << 4);// configure TIM2_CH1
	TIM2->CCMR1 |= OC1PE;

	TIM2->CCMR2 &= ~(7U << 12);
	TIM2->CCMR2 |= (6U << 12);// configure TIM2_CH4
	TIM2->CCMR2 |= OC4PE;

	TIM2->CR1 |= ARPE;

	TIM2->CCER |= CC1E;//enable TIM2_CH1
	TIM2->CCER |= CC1E << 12;//CC4E

	TIM2->EGR |= UG;
	TIM2->CR1 |= CEN;

	enable_driver();
}

void set_period(uint32_t period)
{
	if(period < 200)
	{
		period = 200;
	}

	TIM2->ARR = period - 1;
}

void set_direction(uint16_t direction)
{
	if(direction == FORWARD)
	{
		GPIOA->BSRR = (1U << DIR_PIN_1) | (1U << (16 + DIR_PIN_2));
	}
	else
	{
		GPIOA->BSRR = (1U << DIR_PIN_2) | (1U << (16 + DIR_PIN_1));
	}
}

void set_speed(int16_t speed)
{
	if(speed > 2000)
	{
		speed = 2000;
	}
	else if (speed < -2000)
	{
		speed = -2000;
	}

	target_speed = speed;
}

void apply_speed(int16_t speed)
{
	uint32_t period;
	uint16_t speed_magnitude = speed;
	uint8_t direction;
	if(speed > 0)
	{
		direction = FORWARD;
		speed_magnitude = speed;
	}
	else
	{
		direction = REVERSE;
		speed_magnitude = -1 * speed;
	}

	if (speed_magnitude < 50)
	{
		TIM2->CR1 &= ~CEN;
		TIM2->CNT = 0;
		set_period(65536);
		return;
	}
	else if (!(TIM2->CR1 & CEN))
	{
		period = DISTANCE / speed_magnitude;

		TIM2->CNT = 0;

		set_direction(direction);
		set_period(period);

		TIM2->CR1 |= CEN;
		return;
	}

	period = DISTANCE / speed_magnitude;

	set_direction(direction);
	set_period(period);
}

void update_motor_speed(void)
{
    if (current_speed < target_speed)
    {
        current_speed += ACCEL;

        if (current_speed > target_speed)
            current_speed = target_speed;
    }
    else if (current_speed > target_speed)
    {
        current_speed -= DECEL;

        if (current_speed < target_speed)
            current_speed = target_speed;
    }

    apply_speed(current_speed);
}
