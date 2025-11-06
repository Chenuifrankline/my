/**
 * @file encoder.c
 * @brief Rotary encoder driver with proper wrap-around handling
 */

#include "encoder.h"
#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>

/* Define your limits and initial count */
#define ENCODER_MIN_COUNT 0
#define ENCODER_MAX_COUNT 1000  /* Match display expectation (0-1000 for 12-color palette) */
#define ENCODER_INITIAL_COUNT 0
#define ENCODER_SENSITIVITY 5

/* External variable updated in the task */
extern uint32_t encoderPosition;

/**
 * @brief Get current encoder position with wrap-around handling
 * @return Current encoder count within defined limits
 */
uint32_t encoder_get_position(void) {
    static uint32_t lastRaw = 0;
    static uint8_t initialized = 0;
    uint32_t raw_value = LL_TIM_GetCounter(ENCODER_TIMER);

    if (!initialized) {
        lastRaw = raw_value;
        initialized = 1;
        return encoderPosition;
    }

    int32_t delta = (int32_t)((int16_t)(raw_value - lastRaw));
    delta *= ENCODER_SENSITIVITY;

    int32_t range = (ENCODER_MAX_COUNT - ENCODER_MIN_COUNT + 1);
    int32_t newPosition = (int32_t)encoderPosition + delta;

    /* Wrap-around within [MIN, MAX] */
    while (newPosition < ENCODER_MIN_COUNT) newPosition += range;
    while (newPosition > ENCODER_MAX_COUNT) newPosition -= range;

    lastRaw = raw_value;

    if (newPosition != (int32_t)encoderPosition) {
        printf("Raw: %lu, Delta: %ld, NewPos: %ld\r\n", raw_value, delta, newPosition);
    }

    return (uint32_t)newPosition;
}

/**
 * @brief Reset encoder counter to initial value
 */
void encoder_reset(void) {
    encoderPosition = ENCODER_INITIAL_COUNT;
}

/**
 * @brief Set encoder counter to a specific value (safe)
 * @param value The desired counter value
 */
void encoder_set_counter(uint32_t value) {
    /* Clamp input value to valid range */
    if (value < ENCODER_MIN_COUNT) value = ENCODER_MIN_COUNT;
    if (value > ENCODER_MAX_COUNT) value = ENCODER_MAX_COUNT;

    encoderPosition = value;
}

/**
 * @brief Encoder monitoring task
 * Reads encoder position every 50ms and updates global variable
 * @param args Task arguments (unused)
 */
void encoder_task(void *args) {
    /* Initialize starting position BEFORE any other task reads it */
    encoderPosition = ENCODER_INITIAL_COUNT;

    /* Give other tasks time to see the initial value */
    osDelay(50);

    /* Main task loop */
    for (;;) {
        /* Read and update encoder position */
        uint32_t oldPos = encoderPosition;
        encoderPosition = encoder_get_position();

        /* Print when value changes for debugging */
        if (encoderPosition != oldPos) {
            printf("Enc: %lu -> %lu\r\n", oldPos, encoderPosition);
        }

        /* Wait 50ms before next reading */
        osDelay(50);
    }
}

/**
 * @brief Get encoder direction
 * @return 1 for clockwise, -1 for counter-clockwise, 0 for no movement
 */
int8_t encoder_get_direction(void) {
    static uint32_t lastValue = ENCODER_INITIAL_COUNT;
    uint32_t currentValue = encoderPosition;
    int8_t direction = 0;

    if (currentValue > lastValue) {
        direction = 1;  // Clockwise
    } else if (currentValue < lastValue) {
        direction = -1;  // Counter-clockwise
    }

    lastValue = currentValue;
    return direction;
}
