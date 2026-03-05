#ifndef UI_H
#define UI_H

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "FreeRTOS.h"
#include "task.h"

#include "pico/async_context_freertos.h"

#ifdef __cplusplus
extern "C" {
#endif

void send_data(uint16_t data);
void write_lines(char * lines);
void ui_init();
void ui_worker(async_context_t *context, async_at_time_worker_t *worker);

#ifdef __cplusplus
}
#endif

#endif
