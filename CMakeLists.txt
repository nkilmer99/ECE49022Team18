cmake_minimum_required(VERSION 3.13...3.27)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(pico-sdk/pico_sdk_init.cmake)

project(ECE49022Team18 C CXX ASM)

pico_sdk_init()

include(FreeRTOS_Kernel_import.cmake)

add_executable(main
  src/main.c
  src/ui.c
)

target_include_directories(main PRIVATE
  src
  Config
)

# Linking to FreeRTOS-Kernel-Heap4 to use a dynamic heap for allocations
target_link_libraries(main PRIVATE
  pico_async_context_freertos
  FreeRTOS-Kernel-Heap4
  pico_stdlib
  hardware_spi
)

pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)
pico_add_extra_outputs(main)
