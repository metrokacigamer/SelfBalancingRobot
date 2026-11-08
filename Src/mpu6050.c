#include "mpu6050.h"
#include "systick.h"
#include <math.h>

mag_calib_t mag_cal = {0};

//Barometer Calibration

uint8_t OSS = 0x3U;

float P0 = 101325.0f;

int16_t AC1, AC2, AC3, B1, B2, MB, MC, MD;
uint16_t AC4, AC5, AC6;

int32_t B3, B5, B6, UT, UP, X1, X2, X3, T, P;
uint32_t B4, B7;

static const uint8_t conversion_time_ms[] =
{
    5, 8, 14, 26
};

char data_gy;

//volatile uint8_t ac_data[6];

void gy_read(uint8_t reg)
{
	I2C1_byte_read(AC_SAD, reg, &data_gy);
}

void gy_write(uint8_t reg, char value)
{
	char data[1];
	data[0] = value;
	I2C1_burst_write(AC_SAD, reg, 1, data);
}

void gy_ac_read_values(uint8_t* ac_data)
{
	I2C1_burst_read(AC_SAD, AC0_REG, 6, ac_data);
}

void gy_ac_init()
{
	//enable I2C1 on STM
	I2C1_init();

	uint8_t ctrl1;

	gy_read(CTRL_REG1);
	ctrl1 = data_gy;

	// enable X/Y/Z + 100Hz (example)
	ctrl1 |= 0x07;
	ctrl1 |= (0x10);   // example data rate bits

	gy_write(CTRL_REG1, ctrl1 | (FREQ_1HZ << 4));
	//set 4G
	gy_read(CTRL_REG4);

	data_gy |= (FOUR_G << 4);
	data_gy &= ~(1U << 5);

	gy_write(CTRL_REG4, data_gy);

	//set freq to 1HZ - to make sure it all works;

}

void mag_init(void)
{
    I2C1_init();

    // 1. Data rate = 15 Hz
    uint8_t cra = 0x14;
    I2C1_burst_write(MG_SAD, MAG_CRA_REG, 1, &cra);

    // 2. Gain = default
    uint8_t crb = 0x20;
    I2C1_burst_write(MG_SAD, MAG_CRB_REG, 1, &crb);

    // 3. Continuous conversion mode
    uint8_t mr = MAG_CONTINUOUS;
    I2C1_burst_write(MG_SAD, MAG_MR_REG, 1, &mr);
}

void mag_read_raw(int16_t *x, int16_t *y, int16_t *z)
{
    uint8_t data[6];

    I2C1_burst_read(MG_SAD, MG0_REG, 6, data);

    *x = (int16_t)(data[0] << 8 | data[1]);
    *z = (int16_t)(data[2] << 8 | data[3]);
    *y = (int16_t)(data[4] << 8 | data[5]);
}

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

void mpu_read(int16_t *x, int16_t *y, int16_t *z)
{
	uint8_t buf[6];

	I2C1_burst_read(MPU_SAD, 0x3B, 6, buf);

	*x = (buf[0] << 8) | buf[1];
	*y = (buf[2] << 8) | buf[3];
	*z = (buf[4] << 8) | buf[5];
}

void mag_calibrate(uint16_t samples)
{
    int16_t mx, my, mz;

    int16_t xmin = 32767, xmax = -32768;
    int16_t ymin = 32767, ymax = -32768;
    int16_t zmin = 32767, zmax = -32768;

    for(uint16_t i = 0; i < samples; i++)
    {
    	mag_read_raw(&mx, &my, &mz);

        if(mx < xmin) xmin = mx;
        if(mx > xmax) xmax = mx;

        if(my < ymin) ymin = my;
        if(my > ymax) ymax = my;

        if(mz < zmin) zmin = mz;
        if(mz > zmax) zmax = mz;

        systickDelayMs(20);
    }

    mag_cal.x_offset = (xmax + xmin) / 2.0f;
    mag_cal.y_offset = (ymax + ymin) / 2.0f;
    mag_cal.z_offset = (zmax + zmin) / 2.0f;
}

void mag_get_calibrated(float *mx, float *my, float *mz)
{
    int16_t rx, ry, rz;

    mag_read_raw(&rx, &ry, &rz);

    *mx = rx - mag_cal.x_offset;
    *my = ry - mag_cal.y_offset;
    *mz = rz - mag_cal.z_offset;
}

void barometer_read_values(void)
{
	uint8_t byte_1, byte_2;

    I2C1_burst_read(BARO_SAD, 0xAA, 1, &byte_1);
    I2C1_burst_read(BARO_SAD, 0xAB, 1, &byte_2);

    AC1 = byte_1 << 8 | byte_2;

    I2C1_burst_read(BARO_SAD, 0xAC, 1, &byte_1);
    I2C1_burst_read(BARO_SAD, 0xAD, 1, &byte_2);

    AC2 = byte_1 << 8 | byte_2;

    I2C1_burst_read(BARO_SAD, 0xAE, 1, &byte_1);
    I2C1_burst_read(BARO_SAD, 0xAF, 1, &byte_2);

    AC3 = byte_1 << 8 | byte_2;

    I2C1_burst_read(BARO_SAD, 0xB0, 1, &byte_1);
    I2C1_burst_read(BARO_SAD, 0xB1, 1, &byte_2);

    AC4 = byte_1 << 8 | byte_2;

    I2C1_burst_read(BARO_SAD, 0xB2, 1, &byte_1);
    I2C1_burst_read(BARO_SAD, 0xB3, 1, &byte_2);

    AC5 = byte_1 << 8 | byte_2;

    I2C1_burst_read(BARO_SAD, 0xB4, 1, &byte_1);
    I2C1_burst_read(BARO_SAD, 0xB5, 1, &byte_2);

    AC6 = byte_1 << 8 | byte_2;

    I2C1_burst_read(BARO_SAD, 0xB6, 1, &byte_1);
    I2C1_burst_read(BARO_SAD, 0xB7, 1, &byte_2);

    B1 = byte_1 << 8 | byte_2;

    I2C1_burst_read(BARO_SAD, 0xB8, 1, &byte_1);
    I2C1_burst_read(BARO_SAD, 0xB9, 1, &byte_2);

    B2 = byte_1 << 8 | byte_2;

    I2C1_burst_read(BARO_SAD, 0xBA, 1, &byte_1);
    I2C1_burst_read(BARO_SAD, 0xBB, 1, &byte_2);

    MB = byte_1 << 8 | byte_2;
    I2C1_burst_read(BARO_SAD, 0xBC, 1, &byte_1);
    I2C1_burst_read(BARO_SAD, 0xBD, 1, &byte_2);

    MC = byte_1 << 8 | byte_2;

    I2C1_burst_read(BARO_SAD, 0xBE, 1, &byte_1);
    I2C1_burst_read(BARO_SAD, 0xBF, 1, &byte_2);

    MD = byte_1 << 8 | byte_2;
}

void barometer_calibrate(void)
{
	// Block #1
	uint8_t byte_1, byte_2, byte_3;

	uint8_t data = 0x2E;
	I2C1_burst_write(BARO_SAD, 0xF4, 1, &data);

    systickDelayMs(5);

	I2C1_burst_read(BARO_SAD, 0xF6, 1, &byte_1);
	I2C1_burst_read(BARO_SAD, 0xF7, 1, &byte_2);

	UT = ((uint16_t)byte_1 << 8) | byte_2;

	data = OSS << 6 | 0x34;

	// Block #2

	I2C1_burst_write(BARO_SAD, 0xF4, 1, &data);

	systickDelayMs(conversion_time_ms[OSS]);

	I2C1_burst_read(BARO_SAD, 0xF6, 1, &byte_1);
	I2C1_burst_read(BARO_SAD, 0xF7, 1, &byte_2);
	I2C1_burst_read(BARO_SAD, 0xF8, 1, &byte_3);

	UP = (((uint32_t)byte_1 << 16) |
		  ((uint32_t)byte_2 << 8) |
		   (uint32_t)byte_3) >> (0x8U - OSS); //8 - OSS

	//Block #3

	X1 = (UT - AC6) * AC5 / 32768;
	X2 = MC * 2048 / (X1 + MD);
	B5 = X1 + X2;
	T = (B5 + 8) / 16;

	// Block #4

	B6 = B5 - 4000;
	X1 = (B2 * (B6 * B6 / 4096)) / 2048;
	X2 = AC2 * B6 / 2048;
	X3 = X1 + X2;
	B3 = (((AC1 * 4 + X3) << OSS) + 2) / 4;
	X1 = AC3 * B6 / 8192;
	X2 = (B1 * (B6 * B6 / 4096)) / 65536;
	X3 = ((X1 + X2) + 2) / 4;
	B4 = AC4 * (uint32_t)(X3 + 32768) / 32768;
	B7 = ((uint32_t)UP - B3) * (50000 >> OSS);

	if (B7 < 0x80000000)
	{
		P = (B7 * 2) / B4;
	}
	else
	{
		P = (B7 / B4) * 2;
	}

	X1 = (P / 256) * (P / 256);
	X1 = (X1 * 3038) / 65536;
	X2 = (-7357 * P) / 65536;
	P = P + (X1 + X2 + 3791) / 16;
}

void convert_altitude(float* altitude)
{
	*altitude = 44330.0f * (1.0f - powf((P * 1.0f) / P0, 0.190294f));
}

void baro_init(void)
{
    I2C1_init();

    uint8_t data = 0;
    while(data != 0x55)
    {
    	I2C1_burst_read(BARO_SAD, 0xD0, 1, &data);
    }


	barometer_read_values();
}


