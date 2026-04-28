#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "prod_screen.h"

#include "motor_control.h"
#include "weight_sensor.h"
#include "read_temp.h"
#include "pid_temp.h"

#include "pico/async_context_freertos.h"
#include "FreeRTOS.h"
#include "lvgl.h"

#define TOP_PAD 3
#define BUTTON_DIFF 40
#define TEXTAREA_DIFF 20
#define LABEL_DIFF 18

#define LINE_BUF_SIZE 100

enum {
  PROD_SCREEN_INIT = 0,
  PROD_SCREEN_PRESET,
  PROD_SCREEN_CUSTOM,
  PROD_SCREEN_WARMUP,
  PROD_SCREEN_COOKING,
  PROD_SCREEN_ERROR,
  PROD_SCREEN_DONE,
  PROD_SCREEN_SIZE
};

struct screen_t {
  lv_obj_t * screen;
  lv_obj_t * root;
  lv_obj_t ** objects;
  char ** bufs;
  uint32_t num_objects;
};

uint32_t cursor = 0;
uint32_t prod_scroll_pos = 0;

struct screen_t * prod_screens = NULL;
lv_style_t global_style;

uint32_t current_screen = PROD_SCREEN_INIT;
uint32_t end_tick = 0;

void prod_screen_init() {
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

  // Init screens
  prod_screens = pvPortMalloc(sizeof(struct screen_t) * PROD_SCREEN_SIZE);

  // Setup screens and roots
  for (int i = 0; i < PROD_SCREEN_SIZE; i++) {
    prod_screens[i].screen = lv_obj_create(NULL);
    prod_screens[i].root = lv_obj_create(prod_screens[i].screen);
    lv_obj_add_style(prod_screens[i].root, &global_style, 0);

    prod_screens[i].objects = NULL;
    prod_screens[i].bufs = NULL;
    prod_screens[i].num_objects = 0;
  }

  // Init individual screens
  printf("Init screens!\n");
  init_n_button(PROD_SCREEN_INIT, 2);
  init_n_button(PROD_SCREEN_PRESET, 10);
  init_n_input(PROD_SCREEN_CUSTOM, 2);
  init_n_label(PROD_SCREEN_WARMUP, 4);
  init_n_label(PROD_SCREEN_COOKING, 4);
  init_n_label(PROD_SCREEN_DONE, 2);
  init_n_label(PROD_SCREEN_ERROR, 2);
}

void init_n_button(uint32_t screen, uint32_t n_buttons) {
  prod_screens[screen].objects = pvPortMalloc(sizeof(lv_obj_t *) * 2 * n_buttons);
  prod_screens[screen].num_objects = 2 * n_buttons;
  prod_screens[screen].bufs = pvPortMalloc(sizeof(char *) * 2 * n_buttons);

  for (int i = 0; i < n_buttons; i++) {
    lv_obj_t * button = lv_button_create(prod_screens[screen].root);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, (BUTTON_DIFF * i) + TOP_PAD);

    lv_obj_t * label = lv_label_create(button);
    lv_label_set_text_fmt(label, "Button %d", i);
    lv_obj_center(label);

    /*
    lv_obj_add_style(textarea, &small_style, 0);
    lv_obj_set_size(textarea, 124, 18);
    */

    prod_screens[screen].objects[2 * i] = button;
    prod_screens[screen].objects[(2 * i) + 1] = label;

    prod_screens[screen].bufs[2 * i] = pvPortMalloc(sizeof(char) * LINE_BUF_SIZE);
    prod_screens[screen].bufs[(2 * i) + 1] = pvPortMalloc(sizeof(char) * LINE_BUF_SIZE);
  }
}

void init_n_label(uint32_t screen, uint32_t n_labels) {
  prod_screens[screen].objects = pvPortMalloc(sizeof(lv_obj_t *) * n_labels);
  prod_screens[screen].num_objects = n_labels;
  prod_screens[screen].bufs = pvPortMalloc(sizeof(char *) * n_labels);

  for (int i = 0; i < n_labels; i++) {
    lv_obj_t * label = lv_label_create(prod_screens[screen].root);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, (LABEL_DIFF * i) + TOP_PAD);
    lv_label_set_text_fmt(label, "Label %d", i);

    /*
    lv_obj_add_style(textarea, &small_style, 0);
    lv_obj_set_size(textarea, 124, 18);
    */

    prod_screens[screen].objects[i] = label;
    prod_screens[screen].bufs[i] = pvPortMalloc(sizeof(char) * LINE_BUF_SIZE);
  }
}

void init_n_input(uint32_t screen, uint32_t n_inputs) {
  prod_screens[screen].objects = pvPortMalloc(sizeof(lv_obj_t *) * n_inputs);
  prod_screens[screen].num_objects = n_inputs;
  prod_screens[screen].bufs = pvPortMalloc(sizeof(char *) * n_inputs);

  // Make zero padding, small border style
  static lv_style_t small_style;
  lv_style_init(&small_style);
  lv_style_set_radius(&small_style, 0);

  lv_style_set_pad_ver(&small_style, 0);
  lv_style_set_pad_hor(&small_style, 0);

  lv_style_set_border_width(&small_style, 1);

  for (int i = 0; i < n_inputs; i++) {
    lv_obj_t * textarea = lv_textarea_create(prod_screens[screen].root);

    lv_obj_add_style(textarea, &small_style, 0);
    lv_obj_set_size(textarea, 120, 18);

    lv_obj_align(textarea, LV_ALIGN_TOP_MID, 0, (TEXTAREA_DIFF * i) + TOP_PAD);

    lv_textarea_set_placeholder_text(textarea, "Placeholder");
    prod_screens[screen].objects[i] = textarea;

    prod_screens[screen].bufs[i] = pvPortMalloc(sizeof(char) * LINE_BUF_SIZE);
  }
}

int32_t update_n_button(uint32_t screen, char ** bufs, char key) {
  if (bufs != NULL) {
    uint32_t n_bufs = prod_screens[screen].num_objects;
    for (int i = 0; i < n_bufs; i++) {
      if (strcmp(prod_screens[screen].bufs[(2 * i) + 1], bufs[i]) != 0) {
        strcpy(prod_screens[screen].bufs[(2 * i) + 1], bufs[i]);
        lv_label_set_text_static(prod_screens[screen].objects[(2 * i) + 1], prod_screens[screen].bufs[(2 * i) + 1]);
      }
    }
  }

  if (key == '#') {
    lv_obj_remove_state(prod_screens[screen].objects[prod_scroll_pos * 2], LV_STATE_PRESSED);

    prod_scroll_pos = (prod_scroll_pos + 1) % (prod_screens[screen].num_objects / 2);
    lv_obj_scroll_to_view(prod_screens[screen].objects[prod_scroll_pos * 2], NULL);

    lv_obj_add_state(prod_screens[screen].objects[prod_scroll_pos * 2], LV_STATE_PRESSED);
  }
  else if (key == '*') {
    uint32_t pos = prod_scroll_pos;
    lv_obj_remove_state(prod_screens[screen].objects[prod_scroll_pos * 2], LV_STATE_PRESSED);

    prod_scroll_pos = 0;
    lv_obj_scroll_to_view(prod_screens[screen].objects[prod_scroll_pos * 2], NULL);

    lv_obj_add_state(prod_screens[screen].objects[prod_scroll_pos * 2], LV_STATE_PRESSED);

    return pos;
  }

  return -1;
}

void update_n_label(uint32_t screen, char ** bufs, char key) {
  if (bufs != NULL) {
    uint32_t n_bufs = prod_screens[screen].num_objects;
    for (int i = 0; i < n_bufs; i++) {
      if (strcmp(prod_screens[screen].bufs[i], bufs[i]) != 0) {
        strcpy(prod_screens[screen].bufs[i], bufs[i]);
        lv_label_set_text_static(prod_screens[screen].objects[i], prod_screens[screen].bufs[i]);
      }
    }
  }
}

int32_t update_n_input(uint32_t screen, char ** bufs, char key) {
  if (bufs != NULL) {
    uint32_t n_bufs = prod_screens[screen].num_objects;
    for (int i = 0; i < n_bufs; i++) {
      if (strcmp(prod_screens[screen].bufs[i], bufs[i]) != 0) {
        strcpy(prod_screens[screen].bufs[i], bufs[i]);
        lv_textarea_set_placeholder_text(prod_screens[screen].objects[i], prod_screens[screen].bufs[i]);
      }
    }
  }

  switch (key) {
    case '#': lv_obj_remove_state(prod_screens[screen].objects[prod_scroll_pos], LV_STATE_FOCUSED);

              prod_scroll_pos = (prod_scroll_pos + 1) % (prod_screens[screen].num_objects);
              lv_obj_scroll_to_view(prod_screens[screen].objects[prod_scroll_pos], NULL);

              lv_obj_add_state(prod_screens[screen].objects[prod_scroll_pos], LV_STATE_FOCUSED);
              break;
    case '*': key = '.';
    default:  lv_textarea_add_char(prod_screens[screen].objects[prod_scroll_pos], key);
  }
  return -1;
}

void prod_set_active() {
  all_off();
  switch_screen(PROD_SCREEN_INIT);
}

void all_off() {
  motor_control(OFF_MODE);
  set_target_temp(0.0f);
}

void switch_screen(int screen) {
  lv_screen_load(prod_screens[screen].screen);
  current_screen = screen;
}

void prod_update_screen(char key) {
  switch (current_screen) {
    case PROD_SCREEN_INIT:    update_prod_screen_init(key);     break;
    case PROD_SCREEN_PRESET:  update_prod_screen_preset(key);   break;
    case PROD_SCREEN_CUSTOM:  update_prod_screen_custom(key);   break;
    case PROD_SCREEN_WARMUP:  update_prod_screen_warmup(key);   break;
    case PROD_SCREEN_COOKING: update_prod_screen_cooking(key);  break;
    case PROD_SCREEN_ERROR:   update_prod_screen_error(key);    break;
    case PROD_SCREEN_DONE:    update_prod_screen_done(key);     break;
    default: return;
  }
}

void update_prod_screen_init(char key) {
  /*
  char ** bufs = pvPortMalloc(sizeof(char *) * 2);
  for (int i = 0; i < 2; i++) bufs[i] = pvPortMalloc(sizeof(char) * LINE_BUF_SIZE);
  sprintf(bufs[0], "Preset");
  sprintf(bufs[1], "Custom");
  */
  char ** bufs = NULL;

  update_n_label(PROD_SCREEN_WARMUP, bufs, key);
  switch (update_n_button(PROD_SCREEN_INIT, NULL, key)) {
    case 0: switch_screen(PROD_SCREEN_PRESET); break;
    case 1: switch_screen(PROD_SCREEN_CUSTOM); //break;
    default: return;
  }
}

void update_prod_screen_preset(char key) {
  // TODO: handle option selection

  if (update_n_button(PROD_SCREEN_PRESET, NULL, key) >= 0) {
    switch_screen(PROD_SCREEN_WARMUP);
  }
}

void update_prod_screen_custom(char key) {
  // TODO: handle option input

}

void update_prod_screen_warmup(char key) {
  char ** bufs = pvPortMalloc(sizeof(char *) * 4);
  for (int i = 0; i < 4; i++) bufs[i] = pvPortMalloc(sizeof(char) * LINE_BUF_SIZE);
  sprintf(bufs[0], "Warmup");
  sprintf(bufs[1], "Temp: %.2f", get_temp());
  sprintf(bufs[2], "Time: ");
  sprintf(bufs[3], "* to cancel");
  update_n_label(PROD_SCREEN_WARMUP, bufs, key);

  if (key == '*') {
    switch_screen(PROD_SCREEN_DONE);
    all_off();
  } else if (get_temp() >= get_target_temp()) {
    switch_screen(PROD_SCREEN_COOKING);
  }
}

void update_prod_screen_cooking(char key) {
  uint32_t tick_count = xTaskGetTickCount();
  char ** bufs = pvPortMalloc(sizeof(char *) * 4);
  for (int i = 0; i < 4; i++) bufs[i] = pvPortMalloc(sizeof(char) * LINE_BUF_SIZE);
  sprintf(bufs[0], "Cooking");
  sprintf(bufs[1], "Temp: %.2f", get_temp());
  sprintf(bufs[2], "Time: ");
  sprintf(bufs[3], "* to cancel");
  update_n_label(PROD_SCREEN_COOKING, bufs, key);

  if (key == '*' || tick_count >= end_tick) {
    switch_screen(PROD_SCREEN_DONE);
    all_off();
  }
}

void update_prod_screen_error(char key) {
  char ** bufs = pvPortMalloc(sizeof(char *) * 2);
  for (int i = 0; i < 2; i++) bufs[i] = pvPortMalloc(sizeof(char) * LINE_BUF_SIZE);
  sprintf(bufs[0], "ERROR");
  sprintf(bufs[1], "Restart");
  update_n_label(PROD_SCREEN_ERROR, bufs, key);

  while (1) {
    printf("Error screen\n");
    all_off();
  }
}

void update_prod_screen_done(char key) {
  char ** bufs = pvPortMalloc(sizeof(char *) * 2);
  for (int i = 0; i < 2; i++) bufs[i] = pvPortMalloc(sizeof(char) * LINE_BUF_SIZE);
  sprintf(bufs[0], "Done Cooking");
  sprintf(bufs[1], "Press *");
  update_n_label(PROD_SCREEN_DONE, bufs, key);

  if (key != ' ') switch_screen(PROD_SCREEN_INIT);
}
