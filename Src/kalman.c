#include "kalman.h"

void kalman_1d(float* state, float* uncertainty, float input, float measurement, int32_t period_us)
{
	if (period_us <= 0 || period_us > 20000)
	    return;

	float period_s = (period_us * 1.0f)/1000000.0f;
	float estimate = *state + input * period_s;
	float uncertainty_n = *uncertainty + period_s * period_s * 4.0f * 4.0f;
	float kalman_gain = uncertainty_n / (uncertainty_n + 3.0f * 3.0f);
	estimate = estimate + kalman_gain * (measurement - estimate);
	uncertainty_n = (1.0f - kalman_gain) * uncertainty_n;

	*uncertainty = uncertainty_n;
	*state = estimate;
}
