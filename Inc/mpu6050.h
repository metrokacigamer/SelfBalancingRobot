#ifndef MPU6050_H_
#define MPU6050_H_

#include "i2c.h"

#define MPU_SAD				(0x68)

void mpu_init(void);
void mpu_read(int16_t *x, int16_t *y, int16_t *z);

void mpu_gy_read(int16_t *x, int16_t *y, int16_t *z);
void mpu_gy_init(void);


#endif
