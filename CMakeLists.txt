cmake_minimum_required(VERSION 3.13...3.27)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(pico-sdk/pico_sdk_init.cmake)

project(ECE49022Team18 C CXX ASM)

pico_sdk_init()

include(FreeRTOS_Kernel_import.cmake)

set(PICO_LIBS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pico-libs)
include(pico_libs_import.cmake)
add_subdirectory(pico-libs)
add_subdirectory(pico-libs/src/common/ili934x)
add_subdirectory(pico-libs/src/common/fonts)

set(LV_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Config/lv_conf.h)
add_subdirectory(lvgl)

add_executable(main
  src/main.c
  src/ui.cpp
)

target_include_directories(lvgl PUBLIC
  Config
)

target_include_directories(main PRIVATE
  src
  Config
  ${CMAKE_CURRENT_LIST_DIR}
  ${PICO_LIBS_PATH}/src/common/ili934x
  ${PICO_LIBS_PATH}/src/common/ili934x/include
  ${PICO_LIBS_PATH}/src/common/fonts
  ${PICO_LIBS_PATH}/src/common/fonts/include
)

# Linking to FreeRTOS-Kernel-Heap4 to use a dynamic heap for allocations
target_link_libraries(main PRIVATE
  pico_async_context_freertos
  FreeRTOS-Kernel-Heap4
  pico_stdlib
  hardware_spi
  ili934x
  fonts
  lvgl::lvgl
)

pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)
pico_add_extra_outputs(main)
