#ifndef LED_H_
#define LED_H_

#define STM32F411xE

#include "stm32f4xx.h"

void led_init();

void turn_on_led();
void turn_off_led();
void toggle_led();

#endif /* LED_H_ */
