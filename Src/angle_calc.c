#include "angle_calc.h"
#include "arm_math.h"
#include "math.h"

#define PI               3.14159265358979f

void calculate_angles(float xg, float yg, float zg, float* deg_y, float* deg_x)
{
	float yg_zg, xg_zg;
	arm_sqrt_f32((yg * yg + zg * zg), &yg_zg);
	arm_sqrt_f32((xg * xg + zg * zg), &xg_zg);

	arm_atan2_f32(xg, yg_zg, deg_x);
	arm_atan2_f32(yg, xg_zg, deg_y);
	*deg_x *= -180 * 1.0 / PI;
	*deg_y *= 180 * 1.0 / PI;
}
