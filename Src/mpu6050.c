#include "mpu6050.h"
#include "systick.h"
#include <math.h>

void mpu_init(void)
{
    I2C1_init();

	uint8_t data = 0x00;
	I2C1_burst_write(MPU_SAD, 0x6B, 1, &data);//SIGNAL_PATH_RESET

	data = (1 << 3);
	I2C1_burst_write(MPU_SAD, 0x1C, 1, &data);//ACCEL_CONFIG

	data = 9;
	I2C1_burst_write(MPU_SAD, 0x19, 1, &data);//SMPLRT_DIV
}

void mpu_read(int16_t *x, int16_t *y, int16_t *z)
{
	uint8_t buf[6];

	I2C1_burst_read(MPU_SAD, 0x3B, 6, buf);

	*x = (buf[0] << 8) | buf[1];
	*y = (buf[2] << 8) | buf[3];
	*z = (buf[4] << 8) | buf[5];
}

void mpu_gy_init(void)
{
    I2C1_init();
	uint8_t data;

	I2C1_byte_read(MPU_SAD, 0x75, &data);
	while (data != MPU_SAD)
	{
	    I2C1_byte_read(MPU_SAD, 0x75, &data);
	}

    data = 0x00;
    I2C1_burst_write(MPU_SAD, 0x6B, 1, &data);

    data = 0x03;
    I2C1_burst_write(MPU_SAD, 0x1A, 1, &data);

	data = 0x00;
	I2C1_burst_write(MPU_SAD, 0x1B, 1, &data);

	data = 9;
	I2C1_burst_write(MPU_SAD, 0x19, 1, &data);
}

void mpu_gy_read(int16_t *x, int16_t *y, int16_t *z)
{
	uint8_t buf[6];

	I2C1_burst_read(MPU_SAD, 0x43, 6, buf);

	*x = (buf[0] << 8) | buf[1];
	*y = (buf[2] << 8) | buf[3];
	*z = (buf[4] << 8) | buf[5];
}
