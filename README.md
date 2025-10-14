<!-- omit in toc -->
# Echtzeitsysteme

Echtzeitsysteme Gruppenprojekt von Gruppe F: 
- Jannis Pinkert
- Felix Jung
- Henri Arnold Totue Tagne

<!-- omit in toc -->
## Table of Contents

- [1. Pinout](#1-pinout)
  - [1.1. ST7735 - LCD](#11-st7735---lcd)
- [2. VSCode setup](#2-vscode-setup)
- [3. CMake Setup](#3-cmake-setup)
  - [3.1. 2. Modify `CMakeLists.txt`](#31-2-modify-cmakeliststxt)
    - [3.1.1. a. Add LVGL as a submodule](#311-a-add-lvgl-as-a-submodule)
    - [Exclude default source configuration](#exclude-default-source-configuration)
    - [3.1.2. b. Automatically add all project source files](#312-b-automatically-add-all-project-source-files)
    - [3.1.3. c. Add sources to the executable](#313-c-add-sources-to-the-executable)
    - [3.1.4. d. Add include paths:](#314-d-add-include-paths)
    - [3.1.5. e. Link LVGL to the project:](#315-e-link-lvgl-to-the-project)





##  1. <a name='Pinout'></a>Pinout

###  1.1. <a name='ST7735-LCD'></a>ST7735 - LCD

| PIN  | Function  | Description      |
| :--- | :-------- | :--------------- |
| PB8  | LCD_BLK   | LCD Backlight    |
| PC1  | LCD_MOSI  | LCD SPI MOSI     |
| PC10 | LCD_SCK   | LCD Clock        |
| PC11 | LCD_RESET | LCD Reset        |
| PC12 | LCD_DC    | LCD Data/Command |
| PC12 | LCD_CS    | LCD ChipSelect   |


##  2. <a name='VSCodesetup'></a>VSCode setup

Follow these steps to setup the Stm32 project in VSCode.

1. Install the [STM32CubeCLT](https://www.st.com/en/development-tools/stm32cubeclt.html?icmp=tt38569_gl_lnkon_apr2024) (Command-Line Tools)
2. Install the [STM32CubeIDE for VSCode](https://marketplace.visualstudio.com/items?itemName=stmicroelectronics.stm32-vscode-extension) extension
3. Clone and initialize the project via:
```bash 
git clone --recurse-submodules https://gitlab-fi.ostfalia.de/id705251/ezs-2025-f.git
```
4. Open the project in VSCode via `File > Open Folder`.
5. In the STM32Cube tab, select import project. Leave everything as is and confirm import.


## 3. CMake Setup

CMake has already been configured in this project.
This is just a step by step tutorial in case something breaks.

###  3.1. <a name='ModifyCMakeLists.txt'></a>2. Modify `CMakeLists.txt`

####  3.1.1. <a name='a.AddLVGLasasubmodule'></a>a. Add LVGL as a submodule
Add the following snippet to the CMakeList:
```cmake
# Add LVGL source files 
add_subdirectory(Drivers/lvgl)
```

#### Exclude default source configuration
Add the following snippet to the CMakeList:
```cmake
# Do not auto add folders
set(MX_Application_Src "" CACHE INTERNAL "")
```

####  3.1.2. <a name='b.Automaticallyaddallprojectsourcefiles'></a>b. Automatically add all project source files
Add the following snippet to the CMakeList:
```cmake
# Gather all sources in core/src and lvgl
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/lvgl/src/*.c
)

# Exclude the auto-generated system file
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/system_stm32f4xx\\.c$")
```

####  3.1.3. <a name='c.Addsourcestotheexecutable'></a>c. Add sources to the executable
Find and modify the CMakeList section `Add sources to executable` to look like this:
```cmake
# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${CORE_SOURCES}
)
```

####  3.1.4. <a name='d.Addincludepaths:'></a>d. Add include paths:
Find and modify the CMakeList section `Add include paths` to look like this:
```cmake
# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    Drivers/lvgl
    Core/Inc
)
```

####  3.1.5. <a name='e.LinkLVGLtotheproject:'></a>e. Link LVGL to the project:
Find and modify the CMakeList section `Add linked libraries` to look like this:
```cmake 
# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    # Add user defined libraries
    lvgl
)
```