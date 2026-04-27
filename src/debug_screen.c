#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "debug_screen.h"
#include "display.h"
#include "keys.h"
#include "motor_control.h"
#include "weight_sensor.h"
#include "read_temp.h"
#include "pid_temp.h"

#include "FreeRTOS.h"
#include "lvgl.h"

#define LABEL_HEIGHT 10
#define LABEL_PADDING 8
#define LABEL_DIFF (LABEL_HEIGHT + LABEL_PADDING)
#define MAX_LINES (HEIGHT / LABEL_DIFF)

#define AREA_HEIGHT 18
#define AREA_PADDING 0
#define AREA_DIFF (AREA_HEIGHT + AREA_PADDING)

#define LINE_BUF_SIZE 100 // 100

lv_obj_t * debug_screen = NULL;

struct line {
  char * buf;
  size_t cursor;

  lv_obj_t * lv_obj;
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
  INPUTS_SIZE
};

uint32_t debug_scroll_pos = 0;
char * tmp_buf;
bool dot = false;

void print_xHeapStats() {
  // Print heap stats
  struct xHeapStats heapStats;
  vPortGetHeapStats(&heapStats);
  printf("Heap stats:\n");
  printf("xAvailableHeapSpaceInBytes: %zu\n", heapStats.xAvailableHeapSpaceInBytes);
  printf("xSizeOfLargestFreeBlockInBytes: %zu\n", heapStats.xSizeOfLargestFreeBlockInBytes);
  printf("xSizeOfSmallestFreeBlockInBytes: %zu\n", heapStats.xSizeOfSmallestFreeBlockInBytes);
  printf("xNumberOfFreeBlocks: %zu\n", heapStats.xNumberOfFreeBlocks);
  printf("xMinimumEverFreeBytesRemaining: %zu\n", heapStats.xMinimumEverFreeBytesRemaining);
  printf("xNumberOfSuccessfulAllocations: %zu\n", heapStats.xNumberOfSuccessfulAllocations);
  printf("xNumberOfSuccessfulFrees: %zu\n", heapStats.xNumberOfSuccessfulFrees);
}

void debug_screen_init() {
  //Make screen
  debug_screen = lv_obj_create(NULL);

  // Alloc stats
  stats = pvPortMalloc(sizeof(struct line) * STATS_SIZE);

  // Add stats to screen
  for (int i = 0; i < STATS_SIZE; i++) {
    lv_obj_t * label = lv_label_create(debug_screen);
    lv_obj_set_x(label, 2);
    lv_obj_set_y(label, i * LABEL_DIFF);
    lv_label_set_text_fmt(label, "%d", i);

    stats[i].buf = pvPortMalloc(LINE_BUF_SIZE);
    stats[i].cursor = 0;
    stats[i].lv_obj = label;
  }

  // Make zero padding, small border style
  static lv_style_t small_style;
  lv_style_init(&small_style);
  lv_style_set_radius(&small_style, 0);

  lv_style_set_pad_ver(&small_style, 0);
  lv_style_set_pad_hor(&small_style, 0);

  lv_style_set_border_width(&small_style, 1);

  // Alloc inputs
  inputs = pvPortMalloc(sizeof(struct line) * INPUTS_SIZE);

  // Add inputs to screen
  for (int i = 0; i < INPUTS_SIZE; i++) {
    lv_obj_t * textarea = lv_textarea_create(debug_screen);
    lv_obj_set_x(textarea, 2);
    lv_obj_set_y(textarea, (STATS_SIZE * LABEL_DIFF) + (i * AREA_DIFF));
    lv_textarea_set_placeholder_text(textarea, "Placeholder");
    lv_obj_add_style(textarea, &small_style, 0);
    lv_obj_set_size(textarea, 124, 18);

    inputs[i].buf = pvPortMalloc(LINE_BUF_SIZE);
    inputs[i].cursor = 0;
    inputs[i].lv_obj = textarea;
  }

  // Alloc tmp_buf
  tmp_buf = pvPortMalloc(LINE_BUF_SIZE);
}

void debug_set_active() {
  lv_screen_load(debug_screen);
}

void scroll_down() {
  uint32_t states = STATS_SIZE + INPUTS_SIZE - MAX_LINES + 1;
  uint32_t old_pos = debug_scroll_pos;
  debug_scroll_pos = (debug_scroll_pos + 1) % states;
  lv_obj_scroll_to_y(debug_screen, debug_scroll_pos * LABEL_DIFF, NULL);

  // Change focus
  uint32_t old_item = old_pos + MAX_LINES - 1;
  if (old_item < STATS_SIZE) lv_obj_remove_state(stats[old_item].lv_obj, LV_STATE_FOCUSED);
  else lv_obj_remove_state(inputs[old_item - STATS_SIZE].lv_obj, LV_STATE_FOCUSED);

  uint32_t item = debug_scroll_pos + MAX_LINES - 1;
  if (item < STATS_SIZE) lv_obj_add_state(stats[item].lv_obj, LV_STATE_FOCUSED);
  else lv_obj_add_state(inputs[item - STATS_SIZE].lv_obj, LV_STATE_FOCUSED);

  dot = false;
  if (old_item >= STATS_SIZE) {
    inputs[old_item - STATS_SIZE].cursor = 0;
    inputs[old_item - STATS_SIZE].buf[0] = 0;
  }
}

void debug_update_screen(char key) {
  // Update buffers
  for (int i = 0; i < STATS_SIZE; i++) {
    int len = 0;

    switch (i) {
      case STATS_RAW_KEY:   len = sprintf(tmp_buf, "RKEY: %c",   get_raw_key());    break;
      case STATS_KEY:       len = sprintf(tmp_buf, "KEY: %c",    get_key());        break;
      case STATS_WEIGHT:    len = sprintf(tmp_buf, "WGT: %.2f",  get_weight());     break;
      case STATS_TEMP:      len = sprintf(tmp_buf, "TEMP: %.1f", get_temp());       break;
      case STATS_TEMP_P:    len = sprintf(tmp_buf, "P: %.2f",    get_prev_error()); break;
      case STATS_TEMP_I:    len = sprintf(tmp_buf, "I: %.2f",    get_integral());   break;
      case STATS_TEMP_D:    len = sprintf(tmp_buf, "D: %.2f",    get_derivative()); break;
      case STATS_TEMP_PID:  len = sprintf(tmp_buf, "PID: %.1f",  get_pid_output()); break;
      default: continue;
    }

    if (len > 99) printf("ERROR: OVERWRITE TMP_BUF\n");
    if (strcmp(tmp_buf, stats[i].buf) != 0) {
      strcpy(stats[i].buf, tmp_buf);
      lv_label_set_text_static(stats[i].lv_obj, stats[i].buf);
    }
  }

  int32_t item = debug_scroll_pos + MAX_LINES - 1;
  if (item >= STATS_SIZE) item = item - STATS_SIZE;
  else item = -1;

  for (int i = 0; i < INPUTS_SIZE; i++) {
    if (i == item) continue;

    int len = 0;

    switch (i) {
      case INPUTS_MOTOR_MODE:   len = sprintf(tmp_buf, "MOTR: %d",    get_mode());        break;
      case INPUTS_TEMP_TARGET:  len = sprintf(tmp_buf, "TTMP: %.1f",  get_target_temp()); break;
      case INPUTS_TEMP_P:       len = sprintf(tmp_buf, "KP: %.6f",    get_kp());          break;
      case INPUTS_TEMP_I:       len = sprintf(tmp_buf, "Ki: %.6f",    get_ki());          break;
      case INPUTS_TEMP_D:       len = sprintf(tmp_buf, "Kd: %.6f",    get_kd());          break;
      default: continue;
    }

    if (len > 99) printf("ERROR: OVERWRITE TMP_BUF\n");
    if (strcmp(tmp_buf, inputs[i].buf) != 0) {
      strcpy(inputs[i].buf, tmp_buf);
      lv_textarea_set_placeholder_text(inputs[i].lv_obj, inputs[i].buf);
    }
  }

  switch (key) {
    case '#': if (item >= 0 && inputs[item].cursor > 0) {
                // Save
                inputs[item].buf[inputs[item].cursor] = 0; // Null terminator
                switch (item) {
                  case INPUTS_MOTOR_MODE:   motor_control(atoi(inputs[item].buf)); break;
                  case INPUTS_TEMP_TARGET:  set_target_temp(atof(inputs[item].buf)); break;
                  case INPUTS_TEMP_P:       set_kp(atof(inputs[item].buf)); break;
                  case INPUTS_TEMP_I:       set_ki(atof(inputs[item].buf)); break;
                  case INPUTS_TEMP_D:       set_kd(atof(inputs[item].buf)); break;
                  default: break;
                }
                lv_textarea_set_text(inputs[item].lv_obj, "");
              }
              scroll_down();
              item = debug_scroll_pos + MAX_LINES - 1;
              if (item >= STATS_SIZE) {
                item = item - STATS_SIZE;
              }
    case ' ': return;
    case '*': if (item >= 0 && dot) {
                // Cancel
                lv_textarea_set_text(inputs[item].lv_obj, "");
                scroll_down();
                return;
              }
              dot = true;
              key = '.';
    default: break;
  }
  if (item < 0) return;

  lv_textarea_add_char(inputs[item].lv_obj, key);
  inputs[item].buf[inputs[item].cursor++] = key;
}
