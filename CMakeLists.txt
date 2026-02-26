cmake_minimum_required(VERSION 3.14)

# Set project data
set(PROJECT_NAME "SOUS_VIDE_MACHINE")
set(BUILD_NUMBER "1")

# Set app name(s) and version(s)
set(APP_NAME "SOUS_VIDE")
set(APP_VERSION_NUMBER "0.0.0")

# Specify the app(s) source code
set(APP_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/src")

# FROM 1.4.2 -- Set app-side debugging on debug builds
# Now uses CMake generator expression to extract config type
add_compile_definitions($<$<CONFIG:Debug>:DEBUG=1>) # TODO: Verify

# Set env variable 'PICO_SDK_PATH' to the local Pico SDK
# Comment out the set() if you have a global copy of the
# SDK set and $PICO_SDK_PATH defined in your $PATH
set(ENV{PICO_SDK_PATH} "${CMAKE_SOURCE_DIR}/pico-sdk")

# Set flags and directory variables
set(FREERTOS_CFG_DIRECTORY "${CMAKE_SOURCE_DIR}/Config") #TODO: Check if this needs to be a separate directory
#set(FREERTOS_CFG "${CMAKE_SOURCE_DIR}/FreeRTOSConfig.h") #TODO: Check if this needs to be a separate directory
set(FREERTOS_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/FreeRTOS-Kernel")

# Include the Pico SDK
include(pico_sdk_import.cmake) # TODO: Verify

# Name the project
project(${PROJECT_NAME}
        LANGUAGES C CXX ASM
        DESCRIPTION "Souse Vide machine using an RPI Pico and FreeRTOS"
)

# Initialise the Pico SDK
pico_sdk_init()

# Add FreeRTOS as a library
add_library(FreeRTOS STATIC
    ${FREERTOS_SRC_DIRECTORY}/event_groups.c
    ${FREERTOS_SRC_DIRECTORY}/list.c
    ${FREERTOS_SRC_DIRECTORY}/queue.c
    ${FREERTOS_SRC_DIRECTORY}/stream_buffer.c
    ${FREERTOS_SRC_DIRECTORY}/tasks.c
    ${FREERTOS_SRC_DIRECTORY}/timers.c
    ${FREERTOS_SRC_DIRECTORY}/portable/MemMang/heap_3.c
    ${FREERTOS_SRC_DIRECTORY}/portable/GCC/ARM_CM0/port.c
)

# Build FreeRTOS
target_include_directories(FreeRTOS PUBLIC
    ${FREERTOS_CFG_DIRECTORY}/
    #${FREERTOS_CFG}
    ${FREERTOS_SRC_DIRECTORY}/include
    ${FREERTOS_SRC_DIRECTORY}/portable/GCC/ARM_CM0
)

# Include the apps' source code
add_subdirectory(${APP_SRC_DIRECTORY})
