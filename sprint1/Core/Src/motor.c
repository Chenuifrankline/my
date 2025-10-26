/*
 * motor.c
 *
 *  Created on: Oct 20, 2025
 *      Author: chenui
 */


/* src/motor/motor.c */
#include "motor.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_gpio.h"
#include "FreeRTOS.h"
#include "task.h"

/* ======= HARDWARE MAPPING - ADAPT TO YOUR BOARD =======
 * Example placeholders:
 * - Use TIM1 channels for Phase A/B/C PWM (recommended advanced timer)
 * - If you actually only have TIM10 CH1, you must reconfigure another timer with 3 channels.
 */
#ifndef MOTOR_TIM_A
#define MOTOR_TIM_A TIM1   // replace with actual TIM instance
#define MOTOR_TIM_B TIM1
#define MOTOR_TIM_C TIM1
#endif

/* PWM CCR register access helpers (replace as needed) */
static inline void pwm_setA(uint32_t value) { LL_TIM_OC_SetCompareCH1(MOTOR_TIM_A, value); }
static inline void pwm_setB(uint32_t value) { LL_TIM_OC_SetCompareCH2(MOTOR_TIM_B, value); }
static inline void pwm_setC(uint32_t value) { LL_TIM_OC_SetCompareCH3(MOTOR_TIM_C, value); }

/* If you need to force a phase floating/disable, toggle appropriate GPIO/HAL calls */
static inline void phaseA_disable(void) { /* implement depending on hardware */ }
static inline void phaseB_disable(void) { /* implement depending on hardware */ }
static inline void phaseC_disable(void) { /* implement depending on hardware */ }

/* ======= MOTOR STATE ======= */
static motor_values_t mvals = {
    .speed = 0,
    .commutation_delay = MOTOR_COMMUTATION_MAX,
    .duty_cycle_pwm = MOTOR_DUTY_MIN
};

/* 6-step sequence bit pattern mapping (phaseA, phaseB, phaseC)
   Bits: b2 b1 b0 => A B C
   1 = drive (PWM/high), 0 = float/low (depends on H-bridge wiring).
   Sequence chosen to rotate in one direction; invert sequence to reverse.
*/
static const uint8_t stepSequence[6] = { 0b001, 0b101, 0b100, 0b110, 0b010, 0b011 };

/* Clamp helpers */
static inline uint32_t clamp_comm_delay(uint32_t v) {
    if (v < MOTOR_COMMUTATION_MIN) return MOTOR_COMMUTATION_MIN;
    if (v > MOTOR_COMMUTATION_MAX) return MOTOR_COMMUTATION_MAX;
    return v;
}
static inline uint32_t clamp_duty(uint32_t v) {
    if (v < MOTOR_DUTY_MIN) return MOTOR_DUTY_MIN;
    if (v > MOTOR_DUTY_MAX) return MOTOR_DUTY_MAX;
    return v;
}

/* Low-level: apply step: set PWM on phases according to bit pattern */
static void apply_step_bits(uint8_t bits) {
    /* bits: b2=A b1=B b0=C */
    if (bits & 0x4) { pwm_setA(mvals.duty_cycle_pwm); /* enable A */ } else { pwm_setA(0); phaseA_disable(); }
    if (bits & 0x2) { pwm_setB(mvals.duty_cycle_pwm); /* enable B */ } else { pwm_setB(0); phaseB_disable(); }
    if (bits & 0x1) { pwm_setC(mvals.duty_cycle_pwm); /* enable C */ } else { pwm_setC(0); phaseC_disable(); }
}

/* Public API implementations */

void motor_init() {
    // Initialize motor hardware: timers + PWM channels must already be configured in MX_* or here
    // Set initial PWM duty to 0
    pwm_setA(0);
    pwm_setB(0);
    pwm_setC(0);
    // default values already set in mvals
}

int8_t motor_nextStep(void) {
    static uint8_t idx = 0;
    apply_step_bits(stepSequence[idx]);
    idx = (idx + 1) % 6;
    return idx;
}

motor_values_t motor_get_values(void) {
    return mvals;
}

/* Optional functions to change duty/delay at runtime (not in header but recommended) */
void motor_set_duty(uint32_t duty) {
    mvals.duty_cycle_pwm = clamp_duty(duty);
}
void motor_set_commutation_delay(uint32_t delay_ms) {
    mvals.commutation_delay = clamp_comm_delay(delay_ms);
}

/* Task: open-loop start + simple accel profile */
void task_control_motor(void *arguments) {
    motor_init();

    uint32_t delay = mvals.commutation_delay; // start delay
    for (;;) {
        motor_nextStep();
        vTaskDelay(pdMS_TO_TICKS(delay));

        // simple acceleration: reduce delay until minimum
        if (delay > MOTOR_COMMUTATION_MIN) {
            delay -= 1; // tune the decrement for smoothness
            if (delay < MOTOR_COMMUTATION_MIN) delay = MOTOR_COMMUTATION_MIN;
        }
        mvals.commutation_delay = delay;
    }
}
