#ifndef UI_H
#define UI_H

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "FreeRTOS.h"
#include "task.h"

#include "pico/async_context_freertos.h"
#include "motor_control.h"

void ui_init();
void reset_display();
void write_byte(bool dc, uint8_t data);
void send_command(uint8_t * data, size_t len);
void send_data(uint8_t * data, size_t len);
void write_segment(uint8_t * buffer, int x, int y, int w, int h);
char get_key();
void ui_worker(async_context_t *context, async_at_time_worker_t *worker);

#endif
