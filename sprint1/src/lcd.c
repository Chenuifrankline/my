/******************************************************************************
 * @file           : lcd.c
 * @brief          : LCD Display Driver 1.8" 160x128
 ******************************************************************************/

#include "lcd.h"

#include "../../Drivers/lvgl/src/drivers/display/st7735/lv_st7735.h"

// ***************************************************************************

lv_display_t *lcd_display;

static SPI_TypeDef *lcd_spi = NULL;
static DMA_TypeDef *lcd_dma = NULL;
static uint32_t lcd_dma_stream;
static TIM_TypeDef *lcd_backlight_timer = NULL;
static volatile bool lcd_bus_busy;
static uint32_t lcd_timeout_ms;
static uint32_t lcd_error_count;

#define LCD_WIDTH 160
#define LCD_HEIGHT 128

#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /*will be 2 for RGB565 */
#define BUFF_SIZE LCD_HEIGHT * LCD_WIDTH / 10 * BYTE_PER_PIXEL

lv_color_t buf1[BUFF_SIZE];
lv_color_t buf2[BUFF_SIZE];
// ***************************************************************************

static void delay_ms(uint32_t delay){
	uint32_t start = SysTicks;
	while(SysTicks-start >= delay);
	return;
}

static void setCStoHigh()
{
  LL_GPIO_SetOutputPin(LCD_CS_GPIO_Port, LCD_CS_Pin);
}

static void setCStoLow()
{
  LL_GPIO_ResetOutputPin(LCD_CS_GPIO_Port, LCD_CS_Pin);
}

static void setDCtoHigh()
{
  LL_GPIO_SetOutputPin(LCD_DC_GPIO_Port, LCD_DC_Pin);
}

static void setDCtoLow()
{
  LL_GPIO_ResetOutputPin(LCD_DC_GPIO_Port, LCD_DC_Pin);
}

/**
 * Busy wait for tx empty flag with timeout lcd_timeout_ms.
 * Increase lcd_error_count on timeout.
 *
 * @return true if ok and false otherwise
 */
static bool lcd_wait_until_tx_empty() {
  uint32_t start_time = SysTicks;
  while (!LL_SPI_IsActiveFlag_TXE(lcd_spi)) {
    // nothing to do

    // check for timeout
    if ( (SysTicks - start_time) >= lcd_timeout_ms)
    {
      // timeout: CS HIGH to unselect LCD
      setCStoHigh();
      return 0;
    }
  }
  return 1;
}

/**
 * Busy wait for bus busy flag to become "not busy" with timeout lcd_timeout_ms.
 * Increase lcd_error_count on timeout.
 *
 * @return true if ok and false otherwise
 */
static bool lcd_wait_while_busy() {
  uint32_t start_time = SysTicks;
  while (LL_SPI_IsActiveFlag_BSY(lcd_spi)) {
    // nothing to do

    // check for timeout
    if ( (SysTicks - start_time) >= lcd_timeout_ms)
    {
      // timeout: CS HIGH to unselect LCD
      setCStoHigh();
      lcd_error_count++;
      return 0;
    }
  }
  return 1;
}

// ***************************************************************************

void lcd_dma_transfer_complete_cb()
{
  LL_DMA_DisableStream(lcd_dma, lcd_dma_stream);
  LL_SPI_Disable(lcd_spi);
  LL_SPI_DisableDMAReq_TX(lcd_spi);

  setCStoHigh();
  lcd_bus_busy = false;
  lv_display_flush_ready(lcd_display);
}

void lcd_dma_transfer_error_cb()
{
  LL_DMA_DisableStream(lcd_dma, lcd_dma_stream);
  LL_SPI_Disable(lcd_spi);
  LL_SPI_DisableDMAReq_TX(lcd_spi);

  setCStoHigh();
  lcd_bus_busy = false;
  lcd_error_count++;
}

// ***************************************************************************

/**
 * Initialize LCD I/O bus, reset LCD
 */
static void lcd_io_init(void)
{
  // reset the display
  LL_GPIO_ResetOutputPin(LCD_RESET_GPIO_Port, LCD_RESET_Pin);
  delay_ms(120);
  LL_GPIO_SetOutputPin(LCD_RESET_GPIO_Port, LCD_RESET_Pin);
  delay_ms(120);

  // configure CS and DC pins
  setCStoHigh();
  setDCtoHigh();
}


/**
 * Platform-specific implementation of the LCD send command function.
 * In general this should use polling transfer.
 */
static void lcd_send_cmd(lv_display_t * disp,
    const uint8_t * cmd, size_t cmd_size,
    const uint8_t * param, size_t param_size)
{
  // busy wait until the previous transfer is finished
  while(lcd_bus_busy) {
    // nothing to do
	  // TODO add timeout
  }
  LL_SPI_Disable(lcd_spi);
  lcd_wait_while_busy();
  LL_SPI_SetDataWidth(lcd_spi, LL_SPI_DATAWIDTH_8BIT);
  LL_SPI_Enable(lcd_spi);
  lcd_wait_while_busy();
  lcd_wait_until_tx_empty();

  // DC LOW for command, CS LOW to address the display
  setDCtoLow();
  setCStoLow();
  // send the command (8 bit mode)
  for (int index=0; index < cmd_size; index++) {
    LL_SPI_TransmitData8(lcd_spi, cmd[index]);
    if (!lcd_wait_until_tx_empty()) {
      return; // timeout
    }
  }
  lcd_wait_while_busy();

  // send the data (8 bit mode)
  setDCtoHigh();  // DC HIGH for data
  lcd_wait_until_tx_empty();
  for (int index=0; index < param_size; index++) {
    LL_SPI_TransmitData8(lcd_spi, param[index]);
    if (!lcd_wait_until_tx_empty()) {
      return; // timeout
    }
  }
  lcd_wait_while_busy();
  // finish the transfer
  setCStoHigh();  // CS HIGH unselects the display
}

/**
 * Platform-specific implementation of the LCD send color function.
 * For better performance this should use DMA transfer.
 * In case of a DMA transfer a callback must be installed
 * to notify LVGL about the end of the transfer.
 */
static void lcd_send_color(lv_display_t * disp,
    const uint8_t * cmd, size_t cmd_size,
    uint8_t * param, size_t param_size)
{
  // busy wait until the previous transfer is finished
  while(lcd_bus_busy) {
    // nothing to do
    // TODO add timeout
  }

  LL_SPI_Disable(lcd_spi);
  lcd_wait_while_busy();
  LL_SPI_SetDataWidth(lcd_spi, LL_SPI_DATAWIDTH_8BIT);
  LL_SPI_Enable(lcd_spi);
  lcd_wait_while_busy();
  lcd_wait_until_tx_empty();

  // DC LOW for command, CS LOW to address the display
  setDCtoLow();
  setCStoLow();
  //
  // send the command (8 bit mode)
  for (int index=0; index < cmd_size; index++) {
    LL_SPI_TransmitData8(lcd_spi, cmd[index]);
    if (!lcd_wait_until_tx_empty()) {
      return; // timeout
    }
  }
  lcd_wait_while_busy();

  // transfer the cmd's parameters (color data) in 16 bit mode via DMA
  setDCtoHigh();  // DC HIGH for data
  LL_SPI_Disable(lcd_spi);
  lcd_wait_while_busy();
  LL_SPI_SetDataWidth(lcd_spi, LL_SPI_DATAWIDTH_16BIT);
  LL_SPI_Enable(lcd_spi);
  lcd_wait_while_busy();
  lcd_wait_until_tx_empty();
  lcd_bus_busy = 1;
  LL_DMA_ConfigAddresses(lcd_dma, lcd_dma_stream, (uint32_t)param, LL_SPI_DMA_GetRegAddr(lcd_spi), LL_DMA_GetDataTransferDirection(lcd_dma, lcd_dma_stream));
  LL_DMA_SetDataLength(lcd_dma, lcd_dma_stream,param_size/2);


  LL_DMA_EnableStream(lcd_dma, lcd_dma_stream);
  LL_SPI_EnableDMAReq_TX(lcd_spi);  // enables TXDMAEN
  // CS and lcd_bus_busy will be reset in the DMA transfer complete IST
}

// ***************************************************************************

void lcd_init(SPI_TypeDef *spi_base, DMA_TypeDef *dma_base, uint32_t dma_stream, TIM_TypeDef *backlight_timer_base, uint32_t timeout_ms)
{
  lcd_bus_busy = false;
  lcd_timeout_ms = timeout_ms;
  lcd_error_count = 0;

  // configure SPI
  lcd_spi = spi_base;
  LL_SPI_Enable(lcd_spi);
  while(LL_SPI_IsActiveFlag_BSY(lcd_spi));
  // configure DMA
  lcd_dma = dma_base;
  lcd_dma_stream = dma_stream;
  // LL_DMA_ConfigTransfer should be configured in CubeMX
  LL_DMA_EnableIT_TC(lcd_dma, lcd_dma_stream);
  LL_DMA_EnableIT_TE(lcd_dma, lcd_dma_stream);

  // configure PWM for backlight
  lcd_backlight_timer = backlight_timer_base;
  LL_TIM_EnableCounter(lcd_backlight_timer);
  LL_TIM_CC_EnableChannel(lcd_backlight_timer, LL_TIM_CHANNEL_CH1);
  LL_TIM_EnableAllOutputs(lcd_backlight_timer);

  // lvgl initialization
  lv_init();
  lcd_io_init();

  lcd_display = lv_st7735_create(LCD_HEIGHT, LCD_WIDTH, LV_LCD_FLAG_NONE, lcd_send_cmd, lcd_send_color);
  lv_display_set_rotation(lcd_display, LV_DISPLAY_ROTATION_270);
  lv_display_set_buffers(lcd_display, buf1, buf2, BUFF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
}

void lcd_setBacklight(int lcd_backlight_percent)
{
  if(lcd_backlight_percent > 100) lcd_backlight_percent = 100;
  if(lcd_backlight_percent < 0) lcd_backlight_percent = 0;
  uint32_t compare = (uint64_t)(LL_TIM_GetAutoReload(lcd_backlight_timer)/100) * lcd_backlight_percent;
  LL_TIM_OC_SetCompareCH1(lcd_backlight_timer, compare);
}

uint32_t lcd_getErrorCount()
{
  return lcd_error_count;
}

// ***************************************************************************
