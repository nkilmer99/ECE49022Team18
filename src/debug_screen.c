#include <stdio.h>

#include "debug_screen.h"
#include "display.h"

#include "lvgl.h"

lv_obj_t * debug_screen = NULL;

struct line {
  char * buf;
  size_t size;
  size_t cursor;
};

struct line * stats = NULL;

enum {
  STATS_RAW_KEY = 0,
  STATS_KEY,
  STATS_WEIGHT,
  STATS_TEMP,
  STATS_TEMP_P,
  STATS_TEMP_I,
  STATS_TEMP_D,
  STATS_TEMP_PID,
  STATS_SIZE
};

struct line * inputs = NULL;

enum {
  INPUTS_MOTOR_MODE = 0,
  INPUTS_TEMP_TARGET,
  INPUTS_TEMP_P,
  INPUTS_TEMP_I,
  INPUTS_TEMP_D,
  INPUTS_TIMER,
  INPUTS_SIZE
};

#define LABEL_HEIGHT 10
#define LABEL_PADDING 6
#define LABEL_DIFF (LABEL_HEIGHT + LABEL_PADDING)
#define MAX_LINES (HEIGHT / LABEL_DIFF)

#define AREA_HEIGHT 13
#define AREA_PADDING 3
#define AREA_DIFF (AREA_HEIGHT + AREA_PADDING)

uint32_t scroll_pos = 0;

void debug_screen_init() {
  //Make screen
  debug_screen = lv_obj_create(NULL);

  // Make stats
  //stats = pvPortMalloc(sizeof(struct line) * STATS_SIZE);

  // Add stats to screen
  for (int i = 0; i < STATS_SIZE; i++) {
    lv_obj_t * label = lv_label_create(debug_screen);
    lv_obj_set_x(label, 2);
    lv_obj_set_y(label, i * LABEL_DIFF);
    lv_label_set_text_fmt(label, "%d", i);
  }

  // Make zero padding, small border style
  static lv_style_t small_style;
  lv_style_init(&small_style);
  lv_style_set_radius(&small_style, 0);

  lv_style_set_pad_ver(&small_style, 0);
  lv_style_set_pad_hor(&small_style, 0);

  lv_style_set_border_width(&small_style, 1);

  // Add inputs to screen
  for (int i = 0; i < INPUTS_SIZE; i++) {
    lv_obj_t * textarea = lv_textarea_create(debug_screen);
    lv_obj_set_x(textarea, 2);
    lv_obj_set_y(textarea, (STATS_SIZE * LABEL_DIFF) + (i * AREA_DIFF));
    lv_textarea_set_placeholder_text(textarea, "Placeholder");
    lv_obj_add_style(textarea, &small_style, 0);
    lv_obj_set_size(textarea, 124, 16);
  }
}

void debug_set_active() {
  lv_screen_load(debug_screen);
}

void scroll_down() {
  uint32_t states = STATS_SIZE + INPUTS_SIZE - MAX_LINES + 1;
  scroll_pos = (scroll_pos + 1) % states;
  lv_obj_scroll_to_y(debug_screen, scroll_pos * LABEL_DIFF, NULL);

  printf("Scroll: %d!\n", scroll_pos);
}

void debug_update_screen(char key) {
  if (key == '#') {
    scroll_down();
  }
}
