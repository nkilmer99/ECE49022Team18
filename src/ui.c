#include <stdio.h>

#include "ui.h"
#include "display.h"
#include "keys.h"
#include "prod_screen.h"
#include "debug_screen.h"

#define BUF_SIZE WIDTH * HEIGHT * BYTES_PER_PIXEL

bool debug = false;

#define REFRESH_RATE 20 // Every x ms
#define HOLD_TIME 3000 // ms
#define HOLD_THRESH HOLD_TIME / REFRESH_RATE
uint32_t hold_count = 0;

void ui_init() {
  // Keypad init
  keys_init();

  // Display init
  display_init();

  // LVGL setup
  lv_init();
  lv_tick_set_cb(xTaskGetTickCount);

  lv_display_t * display = lv_display_create(128, 160);
  lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565_SWAPPED);

  uint8_t * buf = pvPortMalloc(BUF_SIZE);
  lv_display_set_buffers(display, buf, NULL, BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(display, flush_cb);

  // Init debug and prod
  prod_screen_init();
  debug_screen_init();

  // Set prod as screen
  prod_set_active();
}

void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_buf) {
  write_segment(px_buf, area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1);

  lv_display_flush_ready(disp);
}

void ui_worker(async_context_t *context, async_at_time_worker_t *worker) {
  // Reschedule self for 20 ms in future
  async_context_add_at_time_worker_in_ms(context, worker, 20);

  // Get key
  char key = get_read_key();

  // Check for debug change
  if (get_key() == '#') hold_count++;
  else hold_count = 0;

  if (hold_count > HOLD_THRESH) {
    hold_count = 0;
    debug = !debug;
    if (debug) debug_set_active();
    else prod_set_active();
  }

  // Update screen
  if (debug) debug_update_screen(key);
  else prod_update_screen(key);

  // Update lvgl (writes to display, plays animations, etc.)
  lv_timer_handler();
}
