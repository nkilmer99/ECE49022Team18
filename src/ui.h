#ifndef UI_H
#define UI_H

#include "lvgl.h"
#include "pico/async_context_freertos.h"

void ui_init();
void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_buf);
char get_key();
void ui_worker(async_context_t *context, async_at_time_worker_t *worker);

#endif
