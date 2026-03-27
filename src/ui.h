#ifndef UI_H
#define UI_H

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "FreeRTOS.h"
#include "task.h"

#include "pico/async_context_freertos.h"
#include "motor_control.h"

void ui_init();
void write_byte(bool dc, uint8_t data);
void send_data(bool dc, uint16_t * data, size_t len);
void write_red();
void ui_worker(async_context_t *context, async_at_time_worker_t *worker);

#endif
