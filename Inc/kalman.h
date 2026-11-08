#ifndef KALMAN_H_
#define KALMAN_H_

#include "stdint.h"

void kalman_1d(float* state, float* uncertainty, float input, float measurement, int32_t period_us);

#endif
