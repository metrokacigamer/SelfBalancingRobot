#ifndef TIM2_H_
#define TIM2_H_

#include <stdint.h>

void tim2_init(void);
uint32_t tim2_count(void);


void tim2_1hz_init(void);

uint32_t tim2_cycle_complete(void);

void tim2_clear_UIF(void);

#endif
