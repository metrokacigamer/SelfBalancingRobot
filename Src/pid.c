#include "pid.h"
#include <math.h>
#define I_LIMIT				150.0f


float avg_error = 0.0f;
float D_current = 0.0f;
float D_min = 0.097f;
float D_max = 0.4f;

void UpdateDerivativeGain(float error, float dt)
{
    const float TAU = 0.3f;

    float alpha = dt / (TAU + dt);

    avg_error += alpha * (error - avg_error);

    float factor = fabsf(avg_error) / 7.0f;

    if (factor > 1.0f)
        factor = 1.0f;

    D_current = D_min + D_max * factor;
}

void CalculatePID(float P, float I, float error, float prev_error, float prev_Iterm, float* output)
{
	float dt = 0.001827;;
	UpdateDerivativeGain(error, dt);

	float I_candidate = prev_Iterm + I * ((prev_error + error) * dt * 0.5f);

	float Dterm = D_current * ((error - prev_error) / dt);
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
