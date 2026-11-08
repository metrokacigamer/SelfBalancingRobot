#include "pid.h"

#define 		STM32F411xE

#include "stm32f4xx.h"

void CalculatePID(float P, float I, float D, float error, float prev_error, float prev_Iterm, int16_t* output)
{
	float dt = 0.004f;//4ms
	float Iterm = prev_Iterm + I * ((prev_error + error) * dt/2);
	int16_t pid_output;

	if (Iterm > 2000)
		Iterm = 2000;
	else if (Iterm < -2000)
		Iterm = -2000;

	pid_output = P * error + Iterm + D * ((error - prev_error)/dt);

	*(output++) = pid_output;
	*(output++) = error;
	*output = Iterm;
}
//error =0 -1 * kalmanAngle(unless motor control is added)
