cmake_minimum_required(VERSION 3.13...3.27)

# initialize pico-sdk from submodule
# note: this must happen before project()
include(pico-sdk/pico_sdk_init.cmake)

project(my_project)

pico_sdk_init()

include(FreeRTOS_Kernel_import.cmake)

add_executable(main
  main.c
)

target_include_directories(main PRIVATE
  FreeRTOSConfig
)

# Linking to FreeRTOS-Kernel-Heap4 to use a dynamic heap for allocations
target_link_libraries(main PRIVATE
  pico_async_context_freertos
  FreeRTOS-Kernel-Heap4
  pico_stdlib
)

pico_add_extra_outputs(main)
