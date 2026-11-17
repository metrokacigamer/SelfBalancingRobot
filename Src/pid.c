#include "pid.h"
#include <math.h>
//#define I_LIMIT				50.0f

void CalculatePID(float P, float I, float D, float error, float prev_error, float prev_Iterm, int32_t period_us, float* output)
{
	if (period_us <= 0 || period_us > 20000)
	    return;

	float max_error = fabsf(error);

	if (fabsf(error) > max_error)
	{
		max_error = fabsf(error);
	}
	else
	{
		max_error *= 0.995f;
	}

	float I_LIMIT = 5.0f + 10.0f * max_error;

	float dt = 0.001827f;//(period_us * 1.0f)/1000000.0f;

	float I_candidate = prev_Iterm + I * ((prev_error + error) * dt * 0.5f);
	float d_factor = fabsf(error) / 10.0f;

	if (d_factor > 1.0f)
	    d_factor = 1.0f;

	float D_current = 0.1f + (0.5f - 0.1f) * d_factor;

	float Dterm = D_current * ((error - prev_error) / dt);
//	float Dterm = D * ((error - prev_error)/dt);
	float Pterm = P * error;
	float pid_output;

	float raw = Pterm + I_candidate + Dterm;

	float Iterm = 0.0f;

	if ((raw > -2000.0f && raw < 2000.0f) ||
	    (raw >= 2000.0f && error < 0.0f) ||
	    (raw <= -2000.0f && error > 0.0f))
	{
	    Iterm = I_candidate;
	}
	else
	{
	    Iterm = prev_Iterm;
	}


	if (Iterm > I_LIMIT)
		Iterm = I_LIMIT;
	else if (Iterm < -1.0f * I_LIMIT)
		Iterm = -1.0f * I_LIMIT;


	pid_output = Pterm + Iterm + Dterm;


	*(output++) = pid_output;
	*(output++) = error;
	*output = Iterm;
}
