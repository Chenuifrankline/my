/******************************************************************************
 * @file           : lcd.h
 * @brief          : LCD Display 1.8" 160x128
 ******************************************************************************/


#ifndef INC_LCD_H_
#define INC_LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "lvgl.h"

extern lv_display_t *lcd_display;
extern volatile uint32_t SysTicks;

void lcd_dma_transfer_complete_cb();
void lcd_dma_transfer_error_cb();

void lcd_init(SPI_TypeDef *spi_base, DMA_TypeDef *dma_base, uint32_t dma_stream, TIM_TypeDef *backlight_timer_base, uint32_t timeout_ms);
void lcd_setBacklight(int lcd_backlight_percent);
uint32_t lcd_getErrorCount();

#ifdef __cplusplus
}
#endif

#endif /* INC_LCD_H_ */
