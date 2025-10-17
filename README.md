# EZS-LCD-Display
## Schritt 1: Projekt erstellen
Erstellt ein STM32 Project From Existing STM32CubeMX Configuration File.
Nutzt als Basis die .ioc Datei in diesem Repository.
Diese enthält Voreinstellungen für das Display, die restlichen Komponenten müsst ihr selber konfigurieren.

Kopiert auch die Dateien im `include` und `src` Verzeichnis aus dem Repository in die entsprechenden `Core` Verzeichnisse eures Projekts.

## Schritt 2: LVGL installieren 
  Klont das LVGL Repository bitte in der Version 9.2, damit der Treiber auch sicher funktioniert. \
  ` git clone https://github.com/lvgl/lvgl.git --branch release/v9.2 ` 

  Das Repository klont ihr in den "Drivers" Ordner eures Projekts.
  In den gleichen Ordner kopiert ihr dann, aus dem lvgl Unterordner, auch die Datei lv_conf_template.h. Diese Datei benennt ihr dann in lv_conf.h um.
  In der lv_conf passt ihr dann noch an:
  - oben in der Datei `#if 0` ändern in `#if 1`
  - weiter unten anpassen `#define LV_USE_OS LV_OS_FREERTOS`
  - anpassen: `#define LV_USE_ST7735		1`

  Bitte baut das Projekt durch Rechtsklick auf das Projekt/Run As/STM32 C/C++ Application.
  Sollten beim kompilieren Fehler im Tests Unterordner von lvgl auftreten, könnt ihr diesen Unterordner löschen.

  Danach müsst ihr LVGL zum Include-Pfad hinzufügen: Rechtsklick auf den lvgl-Ordner/Add/Remove include path, Ok.

  Das Projekt muss nun ohne Fehlermeldungen compiliert werden können.

## Schritt 3: Code in stm32f4xx_it.c anpassen
In der stm32f4xx_it.c müsst ihr folgende Anpassungen vornehmen:
```c
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "./src/tick/lv_tick.h"
/* USER CODE END Includes */
```

 ```c
 /* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile uint32_t SysTicks = 0;
/* USER CODE END PV */
```
Die SysTicks-Variable müsst ihr später über eine extern Deklaration an geeigneter Stelle auch in eurer main.h verfügbar machen.

 ```c
 void DMA1_Stream5_IRQHandler(void)
 {
  /* USER CODE BEGIN DMA1_Stream5_IRQn 0 */
	if(LL_DMA_IsActiveFlag_TC5(DMA1)){
		lcd_dma_transfer_complete_cb();
		LL_DMA_ClearFlag_TC5(DMA1);
	}
	if(LL_DMA_IsActiveFlag_TE5(DMA1)){
		lcd_dma_transfer_error_cb();
	}

  /* USER CODE END DMA1_Stream5_IRQn 0 */

  /* USER CODE BEGIN DMA1_Stream5_IRQn 1 */

  /* USER CODE END DMA1_Stream5_IRQn 1 */
 } 
 ```

```c
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */
  lv_tick_inc(1);
  SysTicks++;
  /* USER CODE END TIM2_IRQn 1 */
}
```
## Schritt 4: Code in der main.c anpassen
Nach der Initialisierung der Peripheriegeräte müsst ihr `  LL_SYSTICK_EnableIT();` aufrufen. Außerdem müssen noch folgende Änderungen/Ergänzungen vorgenommen werden:
```c
/* USER CODE BEGIN PFP */
void LVGLTimer(void * argument);
/* USER CODE END PFP */
```
```c
  /* USER CODE BEGIN RTOS_THREADS */
  xTaskCreate(LVGLTimer,"LVGLTimerTask",64,NULL,2,NULL);
  /* USER CODE END RTOS_THREADS */
```
```c
/* USER CODE BEGIN 4 */
void LVGLTimer(void * argument)
{
  for(;;)
  {
    lv_timer_handler();
    vTaskDelay(20);
  }
}
/* USER CODE END 4 */
```
Das Projekt muss nun immer noch ohne Fehlermeldungen compiliert werden können.

## Abschluss

Der LCD Treiber stellt euch unter anderem folgende Funktionen zur Verfügung.
```c 
void lcd_init(SPI_TypeDef *spi_base, DMA_TypeDef *dma_base, uint32_t dma_stream, TIM_TypeDef *backlight_timer_base, uint32_t timeout_ms);
void lcd_setBacklight(int lcd_backlight_percent);
uint32_t lcd_getErrorCount();
```
Mit diesen könnt ihr das Display initialisieren sowie die Display Helligkeit einstellen. \
Der Treibercode muss nicht angepasst werden, damit das Display funktioniert, es steht euch aber frei, den Code zu verbessern oder euren Wünschen anzupassen.

## Pinbelegung
Die fertige Pinbelegung in eurem Projekt könnte so aussehen:
![alt text](image.png)

## Linksammlung
 [https://docs.lvgl.io/9.2/overview/index.html]() \
 [https://docs.lvgl.io/9.2/examples.html]() \
 [https://docs.lvgl.io/9.2/integration/driver/display/lcd_stm32_guide.html#lcd-stm32-guide]() \