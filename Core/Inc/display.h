/**
 * @file display.h
 * @brief LCD display task interface
 */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Main LCD display task
 *
 * Initializes LCD, creates GUI, and updates display at 50Hz
 *
 * @param args Task arguments (unused)
 */
void display_task(void *args);

/**
 * @brief LVGL timer handler task
 *
 * Calls lv_timer_handler() periodically for LVGL processing
 *
 * @param args Task arguments (unused)
 */
void lvgl_timer_task(void *args);

/**
 * @brief Get current encoder position for display
 * @return Current encoder value
 */
uint32_t display_get_encoder_value(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_DISPLAY_H_ */
