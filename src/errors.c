#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

void vApplicationMallocFailedHook(void) {
    taskDISABLE_INTERRUPTS();

    printf("\r\n[FATAL] FreeRTOS Malloc Failed!\r\n");

    // "Halt"
    while(1) { } // Github safe sleep core
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    taskDISABLE_INTERRUPTS();

    printf("\r\n[FATAL] FreeRTOS Stack Overflow Detected!\r\n");
    if (pcTaskName != NULL) {
        printf("Offending Task Name: %s\r\n", pcTaskName);
    }
    printf("Task Handle: %p\r\n", (void*)xTask);

    // "Halt"
    while(1) { }
}
