# STM32F4 LCD + Encoder System

**Echtzeitsysteme Gruppenprojekt von Gruppe M**

## Team Members

- **Nico Hilmer** 
- **Frankline Chenui** 
---

## 📋 Table of Contents

- [Project Overview](#project-overview)
- [Hardware Components](#hardware-components)
- [Pinout](#pinout)
- [Features](#features)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
  - [VSCode Setup](#vscode-setup)
- [Building the Project](#building-the-project)
- [System Architecture](#system-architecture)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)

---

## 🎯 Project Overview

This project implements a real-time embedded system using an STM32F446RE microcontroller with:
- **Rotary Encoder**: Controls color selection through rotation
- **ST7735 LCD Display**: Displays 12 different colors based on encoder position
- **FreeRTOS**: Multi-tasking real-time operating system
- **LVGL**: Graphics library for the display

The system creates an interactive color picker where rotating the encoder changes the display color in real-time.


## 📌 Pinout

### Rotary Encoder Connections

| Encoder Pin | STM32 Pin | Function | Description |
|-------------|-----------|----------|-------------|
| **A** | **PB6** | TIM4_CH1 | Encoder channel A |
| **B** | **PB7** | TIM4_CH2 | Encoder channel B |
| **SW** | **PC8** | GPIO Input | Push button (with pull-down) |
| **VCC** | **3.3V** | Power | Encoder power supply |
| **GND** | **GND** | Ground | Common ground |

### ST7735 LCD Connections

| LCD Pin | STM32 Pin | Function | Description |
|---------|-----------|----------|-------------|
| **MOSI** | **PC1** | SPI3_MOSI | SPI data line |
| **SCK** | **PC10** | SPI3_SCK | SPI clock line |
| **CS** | **PD2** | GPIO Output | Chip select (active low) |
| **DC** | **PC12** | GPIO Output | Data/Command control |
| **RESET** | **PC11** | GPIO Output | LCD reset pin |
| **VCC** | **3.3V** | Power | LCD power supply |
| **GND** | **GND** | Ground | Common ground |
| **Backlight** | **PB8** | TIM10_CH1 | PWM backlight control (optional) |

#

## ✨ Features

- **Real-time Color Selection**: Rotate encoder to cycle through 12 distinct colors
- **Color Name Display**: Shows color name briefly after rotation
- **Smooth Transitions**: 50Hz display update rate for smooth color changes
- **Wrap-around**: Encoder position wraps from 0 to 1000 seamlessly
- **Debouncing**: Noise filtering for stable encoder readings
- **FreeRTOS Multi-tasking**: Separate tasks for encoder and display
- **Debug Output**: Serial debug messages via USART2


### Prerequisites

- **STM32CubeCLT** (Command-Line Tools) - [Download](https://www.st.com/en/development-tools/stm32cubeclt.html)
- **VSCode** with STM32 extension
- **Git** with submodule support
- **Hardware**: STM32F446RE development board, ST7735 LCD, rotary encoder

### Installation

1. **Clone the repository**:
   ```bash
   git clone --recurse-submodules https://gitlab-fi.ostfalia.de/id120794/ezs-2025-m.git
   cd ezs-2025-m
   ```

2. **Open in VSCode**:
   - Open VSCode
   - Go to `File > Open Folder`
   - Select the project directory



## 🏗️ System Architecture

### Software Architecture

```
┌─────────────────────────────────────────┐
│          FreeRTOS Scheduler             │
├─────────────────────────────────────────┤
│                                         │
│  ┌──────────────┐    ┌──────────────┐  │
│  │ encoder_task │    │ display_task │  │
│  │  (Priority 2)│    │  (Priority 3)│  │
│  └──────┬───────┘    └──────┬───────┘  │
│         │                   │          │
│         │ encoderPosition    │          │
│         │ (shared variable)  │          │
│         └──────────┬─────────┘          │
│                    │                    │
└────────────────────┼────────────────────┘
                     │
         ┌───────────┴───────────┐
         │                       │
    ┌────▼────┐            ┌─────▼────┐
    │  TIM4   │            │  SPI3    │
    │ Encoder │            │  LCD     │
    │ Hardware│            │ Hardware │
    └─────────┘            └─────────┘
```

### Task Responsibilities

**encoder_task** (Runs every 50ms):
- Reads TIM4 counter (encoder hardware)
- Calculates position delta
- Updates `encoderPosition` (0-1000)
- Handles wrap-around

**display_task** (Runs every 20ms):
- Reads `encoderPosition`
- Converts position to color (0-1000 → 12 colors)
- Updates LCD display
- Shows color name for 600ms

### Key Variables

- **`encoderPosition`**: Shared variable (0-1000) connecting encoder to display
- **`raw_value`**: Raw TIM4 counter value (0-65535)
- **`delta`**: Change in encoder position since last reading


## 🔍 Troubleshooting

### Display Issues

**Problem**: Display shows nothing
- **Solution**: Check SPI connections (PC1, PC10)
- Verify CS (PD2) is connected
- Check LCD power supply (3.3V)
- Verify backlight is connected (PB8)

**Problem**: Colors not changing
- **Solution**: Check encoder is updating `encoderPosition`
- Verify display_task is running (check serial output)
- Check SPI communication





## 📚 Additional Resources

- [STM32F446RE Datasheet](https://www.st.com/resource/en/datasheet/stm32f446re.pdf)
- [ST7735 Datasheet](https://www.displayfuture.com/pdf/ST7735.pdf)
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)
- [LVGL Documentation](https://docs.lvgl.io/)


## 👥 Credits

**Gruppe M** - Echtzeitsysteme Project
- Nico Hilmer
- Frankline Chenui

