#ifndef SYSTICK_H_
#define SYSTICK_H_
#include <stdint.h>

void systick_1hz_interrupt(void);
void systickDelayMs(int n);
void systickDelayUs(int n);
uint32_t read_calib();

#endif
