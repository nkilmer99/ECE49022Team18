# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13...3.27)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(pico-sdk/pico_sdk_init.cmake)

project(ECE49022Team18 C CXX ASM)

pico_sdk_init()

include(FreeRTOS_Kernel_import.cmake)

set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "")
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "")
add_subdirectory(lvgl)

add_executable(main
  src/main.c
  src/ui.c
  src/motor_control.c
  src/read_temp.c
  src/weight_sensor.c
)

target_include_directories(main PRIVATE
  src
  Config
)

target_include_directories(lvgl PRIVATE
  Config
)

target_link_libraries(lvgl PRIVATE
  FreeRTOS-Kernel-Heap4
)

# Linking to FreeRTOS-Kernel-Heap4 to use a dynamic heap for allocations
target_link_libraries(main PRIVATE
  pico_async_context_freertos
  FreeRTOS-Kernel-Heap4
  pico_stdlib
  hardware_spi
  hardware_pwm
  lvgl
)

pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)
pico_add_extra_outputs(main)
