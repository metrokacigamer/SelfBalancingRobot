#include <stdint.h>
#include "mpu6050.h"
#include <stdio.h>
#include "systick.h"
#include "angle_calc.h"
#include "kalman.h"
#include "stepper_motor.h"
#include "pid.h"
#include "tim3.h"

#define __FPU_PRESENT 1
#define __FPU_USED 1

//target angle
float target = 1.25f;

//stepper motor speed
float speed = 2000.0f;


//GY measurement
int16_t gx, gy, gz;
float xg_gy, yg_gy, zg_gy;

float roll_deg_gy, pitch_deg_gy;
float gx_off = -1037.72f;
float gy_off = 176.06f;
float gz_off = 54.58f;

//ACC measurement
int16_t ax,ay,az;
float xg,yg,zg;

float roll_deg, pitch_deg;
float ax_off = 130.30f;
float ay_off = -243.11f;
float az_off = -200.0f;

//PID constants


//float P_angle = 30.0f, I_angle = 0.0f, D_angle = 0.0f;
float P_angle = 29.0f, I_angle = 85.0f, D_angle = 0.1f;
float error = 0, prev_error = 0, prev_Iterm = 0;

int main(void)
{
	//gyro/angle values into stepper motor control
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
  SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));
#endif
	float PID_outputs[3] = {0.0f,0.0f,0.0f};
	int32_t period = 0;

	float roll = 0, roll_uncertainty = 0;
	uint32_t timer_v1 = 0;

	systickDelayMs(500);

	mpu_init();
	mpu_gy_init();

	systickDelayMs(500);

	motor_init(2000);

	int count = 0;

	tim3_init();
	timer_v1 = tim3_count();
	while(1)
	{
		count++;
		mpu_gy_read(&gx, &gy, &gz);

		xg_gy = ((gx - gx_off) * 1.0 / 131.0f);
		yg_gy = ((gy - gy_off) * 1.0 / 131.0f);
		zg_gy = ((gz - gz_off) * 1.0 / 131.0f);

		mpu_read(&ax, &ay, &az);
		xg = ((ax - ax_off) * 1.0 / 8192.0f);
		yg = ((ay - ay_off) * 1.0 / 8192.0f);
		zg = ((az - az_off) * 1.0 / 8192.0f);

		calculate_angles(xg,yg,zg,&roll_deg,&pitch_deg);
		kalman_1d(&roll, &roll_uncertainty, xg_gy, roll_deg, period);

		error = target - 1.0f * roll;
		CalculatePID(P_angle, I_angle, D_angle, error, prev_error, prev_Iterm, period, PID_outputs);

		speed = PID_outputs[0];
		prev_error = PID_outputs[1];
		prev_Iterm = PID_outputs[2];

		if(roll > 65.0f || roll < -60.0f)
		{
			speed = 0;
		}
		set_speed(speed);
		update_motor_speed();

		period = tim3_count() - timer_v1;
		timer_v1 = tim3_count();
	}
}
