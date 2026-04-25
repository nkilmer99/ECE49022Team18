#include <stdio.h>

#include "ui.h"
#include "display.h"
#include "keys.h"

#define BUF_SIZE WIDTH * HEIGHT * BYTES_PER_PIXEL

lv_obj_t * global_obj = NULL;
lv_obj_t * global_label = NULL;
lv_style_t global_style;

struct line {
  char * buf;
  size_t size;
  size_t cursor;
};

struct line * stats = NULL;

enum {
  STATS_KEY = 0,
  STATS_WEIGHT,
  STATS_TEMP,
  STATS_TEMP_P,
  STATS_TEMP_I,
  STATS_TEMP_D,
  STATS_TEMP_PID,
  STATS_SIZE,
};

struct line * inputs = NULL;

enum {
  INPUTS_MOTOR_MODE = 0,
  INPUTS_TEMP_TARGET,
  INPUTS_TEMP_P,
  INPUTS_TEMP_I,
  INPUTS_TEMP_D,
  INPUTS_TIMER,
};

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

  // Init style
  lv_style_init(&global_style);
  lv_style_set_radius(&global_style, 5);

  lv_style_set_width(&global_style, 128);
  lv_style_set_height(&global_style, 160);

  lv_style_set_pad_ver(&global_style, 10);
  lv_style_set_pad_hor(&global_style, 10);

  lv_style_set_x(&global_style, 0);
  lv_style_set_y(&global_style, 0);

  lv_style_set_bg_opa(&global_style, LV_OPA_COVER);
  static lv_grad_dsc_t grad;
  grad.dir = LV_GRAD_DIR_VER;
  grad.stops_count = 2;
  grad.stops[0].color = lv_palette_lighten(LV_PALETTE_GREY, 1);
  grad.stops[0].opa = LV_OPA_COVER;
  grad.stops[1].color = lv_palette_main(LV_PALETTE_BLUE);
  grad.stops[1].opa = LV_OPA_COVER;

  /*Shift the gradient to the bottom*/
  grad.stops[0].frac  = 80;
  grad.stops[1].frac  = 160;

  lv_style_set_bg_grad(&global_style, &grad);

  // Make object
  global_obj = lv_obj_create(lv_screen_active());
  lv_obj_add_style(global_obj, &global_style, 0);

  // Make label
  global_label = lv_label_create(global_obj);
}

void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_buf) {
  write_segment(px_buf, area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1);

  lv_display_flush_ready(disp);
}

void ui_worker(async_context_t *context, async_at_time_worker_t *worker) {
  // Reschedule self for 20 ms in future
  async_context_add_at_time_worker_in_ms(context, worker, 20);

  // Get key
  char key = get_key();
  printf("Key: %c\n", key);

  // Update key on display
  lv_label_set_text_fmt(global_label, "Key: %c", key);

  // Update lvgl (writes to display, plays animations, etc.)
  lv_timer_handler();
}
