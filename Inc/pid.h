#ifndef PID_H_
#define PID_H_
#include <stdint.h>

void CalculatePID(float P, float I, float error, float prev_error, float prev_Iterm, float* output);


#endif /* PID_H_ */
