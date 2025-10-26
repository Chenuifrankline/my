/*
 * motor.h
 *
 *  Created on: Oct 20, 2025
 *      Author: chenui
 */

#ifndef SRC_MOTOR_MOTOR_H_
#define SRC_MOTOR_MOTOR_H_

#include <stdint.h>
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_gpio.h"

#define MOTOR_COMMUTATION_MAX 10000
#define MOTOR_COMMUTATION_MIN 800
#define MOTOR_DUTY_MAX 840
#define MOTOR_DUTY_MIN 100

typedef struct {
	int32_t speed;
	uint32_t commutation_delay;
	uint32_t duty_cycle_pwm;
} motor_values_t;

void motor_init();
//uint16_t motor_changeDutyCycle(uint16_t value);
//uint32_t motor_changeCommutationDelay(uint32_t value);
//int8_t motor_changeDirection(void);
int8_t motor_nextStep(void);
//int8_t motor_getDirection(void);
//
//void motor_set_speed(int8_t speed);

void task_control_motor(void *arguments);

motor_values_t motor_get_values(void);

#endif /* SRC_MOTOR_MOTOR_H_ */
