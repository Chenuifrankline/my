

#include "display.h"
#include "main.h"
#include "lcd.h"
#include "cmsis_os.h"
#include <stdio.h>

/* External variables */
extern uint32_t encoderPosition;

/* Private variables */
static lv_obj_t *color_area = NULL;
static lv_obj_t *label_color_name = NULL;

/**
 * @brief Convert encoder value (0-1000) to RGB color
 * Creates a smooth rainbow gradient
 */
static lv_color_t encoder_to_color(uint32_t value) {
    /* Map 0-1000 to 0-360 degrees for hue */
    uint16_t hue = (value * 360) / 1000;

    /* Convert HSV to RGB */
    uint16_t  region = hue / 60;
    uint16_t  remainder = (hue % 60) * 255 / 60;
    uint16_t  p = 0;
    uint16_t  q = 255 - remainder;
    uint16_t  t = remainder;

    uint8_t r, g, b;

    switch (region) {
        case 0:  r = 255; g = (uint8_t)t;   b = p;   break;
        case 1:  r = (uint8_t)q;   g = 255; b = p;   break;
        case 2:  r = p;   g = 255; b = (uint8_t)t;   break;
        case 3:  r = p;   g = (uint8_t)q;   b = 255; break;
        case 4:  r = (uint8_t)t;   g = p;   b = 255; break;
        case 5:  r = 255; g = p;   b = (uint8_t)q;   break;
        default: r = 255; g = 0;   b = 0;   break;
    }

    return lv_color_make(r, g, b);
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
    //lv_label_set_text(label_color_name, "Red");
}

/**
 * @brief Update display with current encoder value
 */
static void display_update(void) {
    uint32_t enc_val = encoderPosition;

    /* Update color area */
    if (color_area != NULL) {
        lv_color_t color = encoder_to_color(enc_val);
        lv_obj_set_style_bg_color(color_area, color, LV_PART_MAIN);
    }
//
//    /* Update color name label */
//    if (label_color_name != NULL) {
//        lv_label_set_text(label_color_name, get_color_name(enc_val));
//    }
}

/**
 * @brief Main display task
 * Initializes LCD and GUI, then updates display at 50Hz
 */
void display_task(void *args) {
    /* Initialize LCD */
    lcd_init(SPI3, DMA1, LL_DMA_STREAM_5, TIM10, 100);
    lcd_setBacklight(100);

    /* Create GUI */
    display_create_gui();

    /* Initial render */
    lv_timer_handler();

    /* Wait a bit for encoder task to initialize */
    osDelay(100);

    /* Main display loop - 50Hz update rate */
    uint32_t last_encoder_value = encoderPosition;

    /* Force initial display update */
    display_update();

    for (;;) {
        /* Read current encoder value */
        uint32_t current_encoder = encoderPosition;
        /* Update display only if value changed */
        if (current_encoder != last_encoder_value) {
            printf("Display: Encoder changed from %lu to %lu\r\n",
                   last_encoder_value, current_encoder);
            last_encoder_value = current_encoder;
            display_update();
        }

        /* Always call LVGL handler */
        lv_timer_handler();

        /* Delay 20ms for 50Hz update rate */
        osDelay(20);
    }
}
