<<<<<<< HEAD
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
=======
# EZS-2025-M



## Getting started

To make it easy for you to get started with GitLab, here's a list of recommended next steps.

Already a pro? Just edit this README.md and make it your own. Want to make it easy? [Use the template at the bottom](#editing-this-readme)!

## Add your files

- [ ] [Create](https://docs.gitlab.com/ee/user/project/repository/web_editor.html#create-a-file) or [upload](https://docs.gitlab.com/ee/user/project/repository/web_editor.html#upload-a-file) files
- [ ] [Add files using the command line](https://docs.gitlab.com/topics/git/add_files/#add-files-to-a-git-repository) or push an existing Git repository with the following command:

```
cd existing_repo
git remote add origin https://gitlab-fi.ostfalia.de/id120794/ezs-2025-m.git
git branch -M main
git push -uf origin main
```

## Integrate with your tools

- [ ] [Set up project integrations](https://gitlab-fi.ostfalia.de/id120794/ezs-2025-m/-/settings/integrations)

## Collaborate with your team

- [ ] [Invite team members and collaborators](https://docs.gitlab.com/ee/user/project/members/)
- [ ] [Create a new merge request](https://docs.gitlab.com/ee/user/project/merge_requests/creating_merge_requests.html)
- [ ] [Automatically close issues from merge requests](https://docs.gitlab.com/ee/user/project/issues/managing_issues.html#closing-issues-automatically)
- [ ] [Enable merge request approvals](https://docs.gitlab.com/ee/user/project/merge_requests/approvals/)
- [ ] [Set auto-merge](https://docs.gitlab.com/user/project/merge_requests/auto_merge/)

## Test and Deploy

Use the built-in continuous integration in GitLab.

- [ ] [Get started with GitLab CI/CD](https://docs.gitlab.com/ee/ci/quick_start/)
- [ ] [Analyze your code for known vulnerabilities with Static Application Security Testing (SAST)](https://docs.gitlab.com/ee/user/application_security/sast/)
- [ ] [Deploy to Kubernetes, Amazon EC2, or Amazon ECS using Auto Deploy](https://docs.gitlab.com/ee/topics/autodevops/requirements.html)
- [ ] [Use pull-based deployments for improved Kubernetes management](https://docs.gitlab.com/ee/user/clusters/agent/)
- [ ] [Set up protected environments](https://docs.gitlab.com/ee/ci/environments/protected_environments.html)

***

# Editing this README

When you're ready to make this README your own, just edit this file and use the handy template below (or feel free to structure it however you want - this is just a starting point!). Thanks to [makeareadme.com](https://www.makeareadme.com/) for this template.

## Suggestions for a good README

Every project is different, so consider which of these sections apply to yours. The sections used in the template are suggestions for most open source projects. Also keep in mind that while a README can be too long and detailed, too long is better than too short. If you think your README is too long, consider utilizing another form of documentation rather than cutting out information.

## Name
Choose a self-explaining name for your project.

## Description
Let people know what your project can do specifically. Provide context and add a link to any reference visitors might be unfamiliar with. A list of Features or a Background subsection can also be added here. If there are alternatives to your project, this is a good place to list differentiating factors.

## Badges
On some READMEs, you may see small images that convey metadata, such as whether or not all the tests are passing for the project. You can use Shields to add some to your README. Many services also have instructions for adding a badge.

## Visuals
Depending on what you are making, it can be a good idea to include screenshots or even a video (you'll frequently see GIFs rather than actual videos). Tools like ttygif can help, but check out Asciinema for a more sophisticated method.

## Installation
Within a particular ecosystem, there may be a common way of installing things, such as using Yarn, NuGet, or Homebrew. However, consider the possibility that whoever is reading your README is a novice and would like more guidance. Listing specific steps helps remove ambiguity and gets people to using your project as quickly as possible. If it only runs in a specific context like a particular programming language version or operating system or has dependencies that have to be installed manually, also add a Requirements subsection.

## Usage
Use examples liberally, and show the expected output if you can. It's helpful to have inline the smallest example of usage that you can demonstrate, while providing links to more sophisticated examples if they are too long to reasonably include in the README.

## Support
Tell people where they can go to for help. It can be any combination of an issue tracker, a chat room, an email address, etc.

## Roadmap
If you have ideas for releases in the future, it is a good idea to list them in the README.

## Contributing
State if you are open to contributions and what your requirements are for accepting them.

For people who want to make changes to your project, it's helpful to have some documentation on how to get started. Perhaps there is a script that they should run or some environment variables that they need to set. Make these steps explicit. These instructions could also be useful to your future self.

You can also document commands to lint the code or run tests. These steps help to ensure high code quality and reduce the likelihood that the changes inadvertently break something. Having instructions for running tests is especially helpful if it requires external setup, such as starting a Selenium server for testing in a browser.

## Authors and acknowledgment
Show your appreciation to those who have contributed to the project.

## License
For open source projects, say how it is licensed.

## Project status
If you have run out of energy or time for your project, put a note at the top of the README saying that development has slowed down or stopped completely. Someone may choose to fork your project or volunteer to step in as a maintainer or owner, allowing your project to keep going. You can also make an explicit request for maintainers.
>>>>>>> origin/sprint1
