#ifndef PWM_H_
#define PWM_H_
#include <stdint.h>

uint32_t period_tick_read(void);
uint32_t high_tick_read(void);

float pwm_get_frequency(void);

//void tim5_pwm_duty_set(uint32_t duty);

uint32_t tim5_cnt_val(void);
void tim5_pwm_out_init(uint32_t period, uint32_t initial);
void tim5_pwm_duty_set(uint32_t duty, uint32_t duty_min, uint32_t duty_max, uint32_t min, uint32_t max);

void tim2_pwm_in_init(void);
void tim2_pwm_duty_set(uint32_t duty, uint32_t duty_min, uint32_t duty_max, uint32_t min, uint32_t max);
void tim2_pwm_out_init(uint32_t period, uint32_t initial_duty);
void tim2_toggle_pwm_out_init(uint32_t period_t, uint32_t initial_duty);

void tim3_pwm_duty_set(uint32_t duty, uint32_t duty_min, uint32_t duty_max, uint32_t min, uint32_t max);
void tim3_pwm_out_init(uint32_t period, uint32_t initial_duty);
void tim3_gpioc_4channel_pwm_out_init(uint32_t period, uint32_t initial_duty);

void tim3_gpioc6_pwm_duty_set(uint32_t duty, uint32_t duty_min, uint32_t duty_max, uint32_t min, uint32_t max);
void tim3_gpioc7_pwm_duty_set(uint32_t duty, uint32_t duty_min, uint32_t duty_max, uint32_t min, uint32_t max);
void tim3_gpioc8_pwm_duty_set(uint32_t duty, uint32_t duty_min, uint32_t duty_max, uint32_t min, uint32_t max);
void tim3_gpioc9_pwm_duty_set(uint32_t duty, uint32_t duty_min, uint32_t duty_max, uint32_t min, uint32_t max);

#endif
