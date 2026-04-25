#include "debug_screen.h"

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

void debug_screen_init() {
  //Make screen
  debug_screen = lv_obj_create(NULL);

  // Make stats
  //stats = pvPortMalloc(sizeof(struct line) * STATS_SIZE);

  lv_obj_t * list = lv_list_create(debug_screen);

  for (int i = 0; i < 100; i++) {
    lv_obj_t * label = lv_list_add_text(list, "");
    lv_label_set_text_fmt(label, "%d", i);
  }
}

void debug_set_active() {
  lv_screen_load(debug_screen);
}

void debug_update_screen(char key) {

}
