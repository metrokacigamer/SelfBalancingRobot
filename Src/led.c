#include "led.h"

#define GPIOAEN 			(1U<<0)

#define PIN5 				(1U<<5)
#define LED_PIN 			PIN5

void led_init()
{
	RCC->AHB1ENR |= GPIOAEN;

	GPIOA->MODER |= (1U<<10);
	GPIOA->MODER &= ~(1U<<11);
}

void turn_on_led()
{
	GPIOA->ODR |= LED_PIN;
}

void turn_off_led()
{
	GPIOA->ODR &= ~LED_PIN;
}

void toggle_led()
{
	GPIOA->ODR ^= LED_PIN;
}
