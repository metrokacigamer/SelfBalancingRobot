#ifndef PID_H_
#define PID_H_
#include <stdint.h>

void CalculatePID(float P, float I, float D, float error, float prev_error, float prev_Iterm, int32_t period_us, float* output);


#endif /* PID_H_ */
