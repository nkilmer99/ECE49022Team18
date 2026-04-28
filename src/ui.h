#ifndef UI_H
#define UI_H

#include "lvgl.h"

void ui_init();
void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_buf);
void ui_worker();

#endif
