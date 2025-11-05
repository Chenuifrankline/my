/**
 * @file encoder.h
 * @brief Rotary encoder driver interface
 */

#ifndef INC_ENCODER_H_
#define INC_ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_ll_tim.h"
#include <stdint.h>

/* Defines */
#define ENCODER_TIMER TIM4
#define ENCODER_MAX_COUNT 65535

/* Public Functions */

/**
 * @brief Initialize encoder
 */
void encoder_init(void);

/**
 * @brief Get current encoder position
 * @return Current encoder count value
 */
uint32_t encoder_get_position(void);

/**
 * @brief Reset encoder to zero
 */
void encoder_reset(void);

/**
 * @brief Set encoder counter to specific value
 * @param value New counter value
 */
void encoder_set_counter(uint32_t value);

/**
 * @brief Task function for encoder monitoring
 * @param args Task arguments (unused)
 */
void encoder_task(void *args);

#ifdef __cplusplus
}
#endif

#endif /* INC_ENCODER_H_ */
