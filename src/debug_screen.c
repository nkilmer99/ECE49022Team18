#include <stdio.h>

#include "debug_screen.h"
#include "display.h"

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
  INPUTS_TIMER,
  INPUTS_SIZE
};

uint32_t scroll_pos = 0;
char * tmp_buf;

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
  printf("Debug init!\n");
  //Make screen
  debug_screen = lv_obj_create(NULL);

  // Alloc stats
  printf("Alloc stats!\n");
  stats = pvPortMalloc(sizeof(struct line) * STATS_SIZE);

  // Add stats to screen
  printf("Setting up stats!\n");
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
  printf("Alloc inputs!\n");
  inputs = pvPortMalloc(sizeof(struct line) * STATS_SIZE);

  print_xHeapStats();
  // Add inputs to screen
  printf("Setting up inputs!\n");
  for (int i = 0; i < INPUTS_SIZE; i++) {
    printf("i: %d\n", i);
    printf("textarea\n");
    lv_obj_t * textarea = lv_textarea_create(debug_screen);
    printf("set xy\n");
    lv_obj_set_x(textarea, 2);
    lv_obj_set_y(textarea, (STATS_SIZE * LABEL_DIFF) + (i * AREA_DIFF));
    printf("set placeholder\n");
    lv_textarea_set_placeholder_text(textarea, "Placeholder");
    printf("set style, size\n");
    lv_obj_add_style(textarea, &small_style, 0);
    lv_obj_set_size(textarea, 124, 18);

    printf("Alloc input buf\n");
    inputs[i].buf = pvPortMalloc(LINE_BUF_SIZE);
    printf("Set cursor, textarea\n");
    inputs[i].cursor = 0;
    inputs[i].lv_obj = textarea;
  }

  // Alloc tmp_buf
  printf("Alloc tmp_buf!\n");
  tmp_buf = pvPortMalloc(LINE_BUF_SIZE);
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
