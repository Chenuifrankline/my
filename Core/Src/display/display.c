

#include "display.h"
#include "main.h"
#include "lcd.h"
#include "cmsis_os.h"
#include "lvgl.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

/* External variables */
extern uint32_t encoderPosition;  /* THE KEY VARIABLE - shared with encoder_task */

/* Private variables */
static lv_obj_t *color_area = NULL;      /* Pointer to the screen area that shows color */
static lv_obj_t *label_color_name = NULL; /* Pointer to the text label showing color name */
static uint32_t last_change_ms = 0;      /* Remember when we last changed the display */

/**
 * @brief Convert encoder value (0-1000) to RGB color
 * Quantized to a discrete palette to ensure clearly distinct colors
 */
static const uint32_t PALETTE_HEX[] = {
    0xFF0000, /* Red */
    0xFF7F00, /* Orange */
    0xFFFF00, /* Yellow */
    0x7FFF00, /* Chartreuse */
    0x00FF00, /* Green */
    0x00FFFF, /* Cyan */
    0x007FFF, /* Sky */
    0x0000FF, /* Blue */
    0x7F00FF, /* Purple */
    0xFF00FF, /* Magenta */
    0xFF007F, /* Pink */
    0xFFFFFF  /* White */
};
static const char *PALETTE_NAMES[] = {
    "Red","Orange","Yellow","Chartreuse","Green","Cyan","Sky","Blue","Purple","Magenta","Pink","White"
};

/**
 * Convert encoder position (0-1000) to color index (0-11)
 *
 * HOW IT WORKS:
 * - We have 12 colors in our palette
 * - Encoder position is 0-1000
 * - We divide the range into 12 equal parts:
 *   - 0-83 → Color 0 (Red)
 *   - 84-167 → Color 1 (Orange)
 *   - 168-250 → Color 2 (Yellow)
 *   - ... and so on
 *
 * Example: If encoderPosition = 500
 *   - idx = (500 * 12) / 1001 = 6000 / 1001 = 5 (approximately)
 *   - So we show color index 5 (Cyan)
 */
static inline uint16_t palette_index_from_value(uint32_t value) {
    const uint16_t steps = (sizeof(PALETTE_HEX) / sizeof(PALETTE_HEX[0]));  /* steps = 12 */
    uint16_t idx = (uint16_t)((value * steps) / 1001);  /* Calculate which color (0-11) */
    if (idx >= steps) idx = steps - 1;  /* Safety check: make sure index is valid */
    return idx;
}

/**
 * Convert encoder position to LVGL color
 *
 * HOW IT WORKS:
 * 1. Get the color index from encoder position (0-11)
 * 2. Get the hex color value from PALETTE_HEX array
 * 3. Convert to LVGL color format
 *
 * Example: encoderPosition = 500
 *   - palette_index_from_value(500) = 5
 *   - PALETTE_HEX[5] = 0x00FFFF (Cyan)
 *   - Return cyan color
 */
static lv_color_t encoder_to_color(uint32_t value) {
    return lv_color_hex(PALETTE_HEX[palette_index_from_value(value)]);
}

/**
 * Get color name string from encoder position
 *
 * Example: encoderPosition = 500
 *   - palette_index_from_value(500) = 5
 *   - PALETTE_NAMES[5] = "Cyan"
 *   - Return "Cyan"
 */
static const char* get_color_name(uint32_t value) {
    return PALETTE_NAMES[palette_index_from_value(value)];
}

///**
// * @brief Get color name for current encoder position
// */
//static const char* get_color_name(uint32_t value) {
//    uint16_t hue = (value * 360) / 1000;
//
//    if (hue < 30 || hue >= 330) return "Red";
//    else if (hue < 90) return "Yellow";
//    else if (hue < 150) return "Green";
//    else if (hue < 210) return "Cyan";
//    else if (hue < 270) return "Blue";
//    else return "Magenta";
//}

/**
 * @brief Create and setup GUI elements
 */
static void display_create_gui(void) {
    /* Get active screen */
    lv_obj_t *scr = lv_scr_act();
    lv_obj_remove_style_all(scr);

    /* Set black background */
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* Make screen itself the color area - full screen */
    color_area = scr;

    /* Create color name label - centered on screen */
    label_color_name = lv_label_create(scr);
    lv_obj_center(label_color_name);
    lv_obj_set_style_text_font(label_color_name, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label_color_name, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(label_color_name, "");
}

/**
 * @brief Update display with current encoder value
 *
 * THIS FUNCTION UPDATES THE DISPLAY
 * ==================================
 * HOW IT WORKS:
 * 1. Read encoderPosition (the shared variable from encoder_task)
 * 2. Convert position to color (0-1000 → one of 12 colors)
 * 3. Change the screen background color
 * 4. Show color name for 600ms after rotation
 *
 * STEP-BY-STEP:
 * - encoderPosition = 500 (example)
 * - encoder_to_color(500) → Cyan color
 * - Set screen background to Cyan
 * - Show "Cyan" text on screen for 600ms
 */
static void display_update(void) {
    /* STEP 1: Read the shared variable encoderPosition
     * This is the "mailbox" that encoder_task writes to
     * We read it here to see what the encoder position is
     */
    uint32_t enc_val = encoderPosition;  /* Read current encoder position (0-1000) */

    /* STEP 2: Update the screen background color */
    if (color_area != NULL) {
        /* Convert encoder position to color
         * Example: enc_val = 500 → color = Cyan (0x00FFFF)
         */
        lv_color_t color = encoder_to_color(enc_val);

        /* Change the screen background to this color
         * This is what you actually see on the LCD!
         */
        lv_obj_set_style_bg_color(color_area, color, LV_PART_MAIN);
    }

    /* STEP 3: Update the color name label (text showing color name) */
    if (label_color_name != NULL) {
        /* Show color name for 600ms after rotation
         * This makes the text appear when you rotate, then disappear
         *
         * Example: If you rotated 300ms ago:
         *   - (SysTicks - last_change_ms) = 300
         *   - 300 <= 600, so show the color name
         *
         * Example: If you rotated 700ms ago:
         *   - (SysTicks - last_change_ms) = 700
         *   - 700 > 600, so hide the text (set to empty string "")
         */
        if ((SysTicks - last_change_ms) <= 600) {
            /* Show color name (e.g., "Red", "Blue", "Cyan") */
            lv_label_set_text(label_color_name, get_color_name(enc_val));
        } else {
            /* Hide color name (clear the text) */
            lv_label_set_text(label_color_name, "");
        }
    }
}

/**
 * @brief Main display task
 * Initializes LCD and GUI, then updates display at 50Hz
 *
 * THIS IS THE DISPLAY TASK - IT RUNS IN A LOOP FOREVER
 * =====================================================
 * This task is like a painter that watches the encoder and paints the screen
 *
 * COMPLETE FLOW FROM BEGINNING:
 * ==============================
 *
 * STARTUP:
 * 1. System boots, main() function runs
 * 2. Hardware initialized (TIM4 for encoder, SPI3 for LCD)
 * 3. FreeRTOS scheduler starts
 * 4. Two tasks created: encoder_task and display_task
 *
 * ENCODER TASK (encoder_task):
 * - Runs every 50ms
 * - Reads TIM4 counter (hardware reads encoder signals)
 * - Calculates new position
 * - Updates encoderPosition variable (THE SHARED VARIABLE)
 *
 * DISPLAY TASK (display_task):
 * - Runs every 20ms (50Hz = 50 times per second)
 * - Reads encoderPosition variable (THE SHARED VARIABLE)
 * - Converts position to color (0-1000 → one of 12 colors)
 * - Updates LCD screen to show that color
 * - Shows color name text for 600ms
 *
 * THE CONNECTION:
 * encoderPosition is like a mailbox:
 *   - encoder_task puts new values in (writes)
 *   - display_task reads values from it (reads)
 *   - This connects encoder rotation to screen colors!
 */
void display_task(void *args) {
    /* STEP 1: Initialize the LCD hardware
     * This sets up the SPI communication with the ST7735 display
     * - SPI3: Communication bus
     * - DMA1: Faster data transfer
     * - TIM10: Backlight control
     */
    lcd_init(SPI3, DMA1, LL_DMA_STREAM_5, TIM10, 100);  /* Initialize LCD hardware */
    lcd_setBacklight(100);  /* Turn on backlight to 100% brightness */

    /* STEP 2: Create the GUI (Graphical User Interface)
     * This creates the screen layout:
     *   - Full screen background (color_area)
     *   - Text label in center (label_color_name)
     */
    display_create_gui();

    /* STEP 3: Initial render
     * Draw the screen for the first time
     */
    lv_timer_handler();  /* LVGL library: draw the screen */

    /* STEP 4: Wait for encoder task to start
     * Give encoder_task time to initialize encoderPosition to 0
     */
    osDelay(100);  /* Wait 100ms */

    /* STEP 5: Main loop - runs forever
     * This loop checks the encoder and updates the display
     */
    uint32_t last_encoder_value = encoderPosition;  /* Remember last position we saw */

    /* Force initial display update (show starting color) */
    display_update();  /* Update screen with starting position (probably 0 = Red) */

    /* Main loop - runs forever */
    for (;;) {
        /* STEP 5a: Read the shared variable encoderPosition
         * This is the "mailbox" that encoder_task updates
         * We check if it changed since last time
         */
        uint32_t current_encoder = encoderPosition;  /* Read current encoder position */

        /* STEP 5b: Check if encoder position changed
         * Only update display if position actually changed
         * This saves CPU time and prevents unnecessary screen updates
         */
        if (current_encoder != last_encoder_value) {
            /* Position changed! Print debug message */
            printf("Display: Encoder changed from %lu to %lu\r\n",
                   last_encoder_value, current_encoder);

            /* Remember this new value for next comparison */
            last_encoder_value = current_encoder;

            /* Remember when this change happened (for showing color name for 600ms) */
            last_change_ms = SysTicks;

            /* Update the display (change color and show name) */
            display_update();  /* This is where the magic happens - screen updates! */
        }

        /* STEP 5c: Always call LVGL handler
         * LVGL needs to run regularly to handle animations, timers, etc.
         * This keeps the display library working properly
         */
        lv_timer_handler();  /* LVGL library: process graphics, animations */

        /* STEP 5d: Wait 20ms before checking again
         * 20ms = 50 times per second (50Hz)
         * This is fast enough to feel smooth, but not too fast to waste CPU
         */
        osDelay(20);  /* Wait 20 milliseconds */
    }
    /* This loop never ends - it runs forever, checking encoder and updating display */
}
