<!-- omit in toc -->
# Echtzeitsysteme

Echtzeitsysteme Gruppenprojekt von Gruppe F: 
- Jannis Pinkert
- Felix Jung
- Henri Arnold Totue Tagne

<!-- omit in toc -->
## Table of Contents

- [Pinout](#pinout)
  - [ST7735 - LCD](#st7735---lcd)
- [VSCode setup](#vscode-setup)
- [CMake Integration Instructions:](#cmake-integration-instructions)
  - [1. Modify `cmake/stm32cubemx/CMakeLists.txt`](#1-modify-cmakestm32cubemxcmakeliststxt)
    - [a. Remove application source entries](#a-remove-application-source-entries)
    - [b. Remove system file entry](#b-remove-system-file-entry)
  - [2. Modify `CMakeLists.txt`](#2-modify-cmakeliststxt)
    - [a. Add LVGL as a submodule](#a-add-lvgl-as-a-submodule)
    - [b. Automatically add all project source files](#b-automatically-add-all-project-source-files)
    - [c. Add sources to the executable](#c-add-sources-to-the-executable)
    - [d. Add include paths:](#d-add-include-paths)
    - [e. Link LVGL to the project:](#e-link-lvgl-to-the-project)





##  <a name='Pinout'></a>Pinout

###  <a name='ST7735-LCD'></a>ST7735 - LCD

| PIN  | Function  | Description      |
| :--- | :-------- | :--------------- |
| PB8  | LCD_BLK   | LCD Backlight    |
| PC1  | LCD_MOSI  | LCD SPI MOSI     |
| PC10 | LCD_SCK   | LCD Clock        |
| PC11 | LCD_RESET | LCD Reset        |
| PC12 | LCD_DC    | LCD Data/Command |
| PC12 | LCD_CS    | LCD ChipSelect   |


##  <a name='VSCodesetup'></a>VSCode setup

Follow these steps to setup the Stm32 project in VSCode.

1. Install the [STM32CubeCLT](https://www.st.com/en/development-tools/stm32cubeclt.html?icmp=tt38569_gl_lnkon_apr2024) (Command-Line Tools)
2. Install the [STM32CubeIDE for VSCode](https://marketplace.visualstudio.com/items?itemName=stmicroelectronics.stm32-vscode-extension) extension

3. Clone and initialize the project via:
```bash 
git clone --recurse-submodules https://gitlab-fi.ostfalia.de/id705251/ezs-2025-f.git
```
4. Open the project in VSCode via `File > Open Folder`.


##  <a name='CMakeIntegrationInstructions:'></a>CMake Integration Instructions:

Using CMake allows development via VScode. The following changes need to be made
for CMake to function correctly.

###  <a name='Modifycmakestm32cubemxCMakeLists.txt'></a>1. Modify `cmake/stm32cubemx/CMakeLists.txt`

####  <a name='a.Removeapplicationsourceentries'></a>a. Remove application source entries
In the `set(MX_Application_Src ...)` block, remove all entries referring to:
```cmake 
${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/*.c
```

####  <a name='b.Removesystemfileentry'></a>b. Remove system file entry
In the `set(STM32_Drivers_Src ...)` block, remove the entry:
```cmake 
${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/system_stm32f4xx.c
```



###  <a name='ModifyCMakeLists.txt'></a>2. Modify `CMakeLists.txt`

####  <a name='a.AddLVGLasasubmodule'></a>a. Add LVGL as a submodule
Add the following snippet to the CMakeList:
```cmake
# Add LVGL source files 
add_subdirectory(Drivers/lvgl)
```

####  <a name='b.Automaticallyaddallprojectsourcefiles'></a>b. Automatically add all project source files
Add the following snippet to the CMakeList:
```cmake
# Gather all sources in core/src
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/*.cpp
)
```

####  <a name='c.Addsourcestotheexecutable'></a>c. Add sources to the executable
Find and modify the CMakeList section `Add sources to executable` to look like this:
```cmake
# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${CORE_SOURCES}
)
```

####  <a name='d.Addincludepaths:'></a>d. Add include paths:
Find and modify the CMakeList section `Add include paths` to look like this:
```cmake
# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    Drivers/lvgl
    Core/Inc
)
```

####  <a name='e.LinkLVGLtotheproject:'></a>e. Link LVGL to the project:
Find and modify the CMakeList section `Add linked libraries` to look like this:
```cmake 
# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    # Add user defined libraries
    lvgl
)
```