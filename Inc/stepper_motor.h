#ifndef STEPPER_MOTOR_H_
#define STEPPER_MOTOR_H_
#include <stdint.h>

void motor_init(uint16_t period);
void set_speed(int16_t speed);
void update_motor_speed(void);

#endif /* STEPPER_MOTOR_H_ */
