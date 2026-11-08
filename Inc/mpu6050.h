#ifndef MPU6050_H_
#define MPU6050_H_

#include "i2c.h"

#define AC_SAD 				(0b0011001)
#define MG_SAD				(0b0011110)

#define MPU_SAD				(0x68)
#define BARO_SAD			(0x77)


#define MAG_CRA_REG 		0x00
#define MAG_CRB_REG 		0x01
#define MAG_MR_REG  		0x02

#define MAG_CONTINUOUS 		0x00

#define ACGY_OUT_R			0x3B

#define GYRO_OUT_R			0x43

#define AC0_REG				(0b01010000)
#define MG0_REG				(0b00000011)

#define CTRL_REG4			(0x23)//BLE & Res
#define CTRL_REG1			(0x20)//pow

#define FOUR_G				(0b01)
#define POWER_DOWN			(0x0000)//REG1; reset bits 4-7
#define FREQ_1HZ			(0x0001)//REG1; bits 4-7

typedef struct
{
    float x_offset;
    float y_offset;
    float z_offset;
} mag_calib_t;

extern mag_calib_t mag_cal;

void mag_get_calibrated(float *mx, float *my, float *mz);
void gy_ac_init(void);
void gy_ac_read_values(uint8_t* ac_data);
void mag_init(void);
void mag_read_raw(int16_t *x, int16_t *y, int16_t *z);

void mpu_init(void);
void mpu_read(int16_t *x, int16_t *y, int16_t *z);

void mpu_gy_read(int16_t *x, int16_t *y, int16_t *z);
void mpu_gy_init(void);

void baro_init(void);

void barometer_calibrate(void);
void convert_altitude(float* altitude);

#endif
