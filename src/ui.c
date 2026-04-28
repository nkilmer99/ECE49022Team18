#include <stdio.h>

#include "ui.h"
#include "display.h"
#include "keys.h"
#include "prod_screen.h"
#include "debug_screen.h"
#include "pico/async_context_freertos.h"

#define BUF_SIZE (WIDTH * HEIGHT * BYTES_PER_PIXEL)

#define DEBUG 0

#define REFRESH_RATE 20 // Every x ms
#define HOLD_TIME 3000 // ms
#define HOLD_THRESH (HOLD_TIME / REFRESH_RATE)
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
  uint8_t * buf2 = pvPortMalloc(BUF_SIZE);
  lv_display_set_buffers(display, buf, buf2, BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(display, flush_cb);

  // Init debug and prod
#if DEBUG
  debug_screen_init();
  debug_set_active();
#else
  prod_screen_init();
  prod_set_active();
#endif
}

void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_buf) {
  write_segment(px_buf, area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1);

  lv_display_flush_ready(disp);
}

void ui_worker() {
  // Get key
  char key = get_read_key();

  // Update screen
#if DEBUG
  debug_update_screen(key);
#else
  prod_update_screen(key);
#endif

  // Update lvgl (writes to display, plays animations, etc.)
  lv_timer_handler();
}
