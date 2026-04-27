#include <stdio.h>
#include <string.h>

#include "keys.h"

#include "pico/stdlib.h"

#define KEYS_ROW1 3
#define KEYS_ROW2 1
#define KEYS_ROW3 7
#define KEYS_ROW4 5
#define KEYS_COL1 4
#define KEYS_COL2 2
#define KEYS_COL3 6

#define KEY_DELAY 5 // Poll keypad every X ms
#define KEYS_BUF_SIZE 3 // Number of keys for debouncing
#define KEY_THRESH 2 // Necessary number of elements to consider "pressed"
#define KEY_SLEEP 1 // Sleep time between setting KEYS_col and checking it

#define NUM_KEYS 13
const char KEYS[NUM_KEYS] = {NULL_CHAR, '1', '4', '7', '*', '2' ,'5', '8', '0', '3', '6', '9', '#'};

uint8_t key_buffer[KEYS_BUF_SIZE];
uint8_t key_counts[NUM_KEYS];

bool read = false;

void keys_init() {
  // Keypad init
  gpio_init(KEYS_COL1);
  gpio_set_dir(KEYS_COL1, GPIO_OUT);
  gpio_put(KEYS_COL1, 0);

  gpio_init(KEYS_COL2);
  gpio_set_dir(KEYS_COL2, GPIO_OUT);
  gpio_put(KEYS_COL2, 0);

  gpio_init(KEYS_COL3);
  gpio_set_dir(KEYS_COL3, GPIO_OUT);
  gpio_put(KEYS_COL3, 0);

  gpio_init(KEYS_ROW1);
  gpio_set_dir(KEYS_ROW1, GPIO_IN);
  gpio_pull_down(KEYS_ROW1);

  gpio_init(KEYS_ROW2);
  gpio_set_dir(KEYS_ROW2, GPIO_IN);
  gpio_pull_down(KEYS_ROW2);

  gpio_init(KEYS_ROW3);
  gpio_set_dir(KEYS_ROW3, GPIO_IN);
  gpio_pull_down(KEYS_ROW3);

  gpio_init(KEYS_ROW4);
  gpio_set_dir(KEYS_ROW4, GPIO_IN);
  gpio_pull_down(KEYS_ROW4);

  for (int i = 0; i < KEYS_BUF_SIZE; i++) {
    key_buffer[i] = NULL_CHAR;
  }
}

char get_key() {
  // Get most common key
  uint8_t max_i = 0;
  uint8_t max_c = 0;

  for (int i = 0; i < NUM_KEYS; i++) {
    if (key_counts[i] > max_c) {
      max_c = key_counts[i];
      max_i = i;
    }
  }

  // Check thresh
  if (max_c >= KEY_THRESH) return KEYS[max_i];
  return KEYS[0];
}

char get_read_key() {
  char key = get_key();
  if (key == NULL_CHAR) {
    read = false;
  } else if (read == false) {
    read = true;
    return key;
  }

  return NULL_CHAR;
}

void key_worker(async_context_t *context, async_at_time_worker_t *worker) {
  // Reschedule self
  async_context_add_at_time_worker_in_ms(context, worker, KEY_DELAY);

  // Poll keypad
  uint8_t key = poll_keypad();

  // Update counts
  key_counts[key_buffer[0]]--;
  key_counts[key]++;

  // Append to array
  memcpy(key_buffer, &key_buffer[1], KEYS_BUF_SIZE - 1); // Shift array left 1
  key_buffer[KEYS_BUF_SIZE - 1] = key;

  //printf("%d %d %d %d %d\n", key_buffer[0], key_buffer[1], key_buffer[2], key_buffer[3], key_buffer[4]);
}

char get_raw_key() {
  return KEYS[key_buffer[KEYS_BUF_SIZE - 1]];
}

uint8_t poll_keypad() {
  // KEYS_Col 1
  gpio_put(KEYS_COL1, 1);
  sleep_ms(KEY_SLEEP);
  if (gpio_get(KEYS_ROW1)) {
    gpio_put(KEYS_COL1, 0);
    return 1;
  } else if (gpio_get(KEYS_ROW2)) {
    gpio_put(KEYS_COL1, 0);
    return 2;
  } else if (gpio_get(KEYS_ROW3)) {
    gpio_put(KEYS_COL1, 0);
    return 3;
  } else if (gpio_get(KEYS_ROW4)) {
    gpio_put(KEYS_COL1, 0);
    return 4;
  }
  gpio_put(KEYS_COL1, 0);

  // KEYS_Col 2
  gpio_put(KEYS_COL2, 1);
  sleep_ms(KEY_SLEEP);
  if (gpio_get(KEYS_ROW1)) {
    gpio_put(KEYS_COL2, 0);
    return 5;
  } else if (gpio_get(KEYS_ROW2)) {
    gpio_put(KEYS_COL2, 0);
    return 6;
  } else if (gpio_get(KEYS_ROW3)) {
    gpio_put(KEYS_COL2, 0);
    return 7;
  } else if (gpio_get(KEYS_ROW4)) {
    gpio_put(KEYS_COL2, 0);
    return 8;
  }
  gpio_put(KEYS_COL2, 0);

  // KEYS_Col 3
  gpio_put(KEYS_COL3, 1);
  sleep_ms(KEY_SLEEP);
  if (gpio_get(KEYS_ROW1)) {
    gpio_put(KEYS_COL3, 0);
    return 9;
  } else if (gpio_get(KEYS_ROW2)) {
    gpio_put(KEYS_COL3, 0);
    return 10;
  } else if (gpio_get(KEYS_ROW3)) {
    gpio_put(KEYS_COL3, 0);
    return 11;
  } else if (gpio_get(KEYS_ROW4)) {
    gpio_put(KEYS_COL3, 0);
    return 12;
  }
  gpio_put(KEYS_COL3, 0);

  return 0;
}

void debug_print_pins() {
  printf("Keypad pins:\n");
  printf("KEYS_COL1: Func: %d, Dir: %d, PDown: %d, PUp: %d, Val: %d\n", gpio_get_function(KEYS_COL1), gpio_get_dir(KEYS_COL1), gpio_is_pulled_down(KEYS_COL1), gpio_is_pulled_up(KEYS_COL1), gpio_get(KEYS_COL1));
  printf("KEYS_COL2: Func: %d, Dir: %d, PDown: %d, PUp: %d, Val: %d\n", gpio_get_function(KEYS_COL2), gpio_get_dir(KEYS_COL2), gpio_is_pulled_down(KEYS_COL2), gpio_is_pulled_up(KEYS_COL2), gpio_get(KEYS_COL2));
  printf("KEYS_COL3: Func: %d, Dir: %d, PDown: %d, PUp: %d, Val: %d\n", gpio_get_function(KEYS_COL3), gpio_get_dir(KEYS_COL3), gpio_is_pulled_down(KEYS_COL3), gpio_is_pulled_up(KEYS_COL3), gpio_get(KEYS_COL3));
  printf("KEYS_ROW1: Func: %d, Dir: %d, PDown: %d, PUp: %d, Val: %d\n", gpio_get_function(KEYS_ROW1), gpio_get_dir(KEYS_ROW1), gpio_is_pulled_down(KEYS_ROW1), gpio_is_pulled_up(KEYS_ROW1), gpio_get(KEYS_ROW1));
  printf("KEYS_ROW2: Func: %d, Dir: %d, PDown: %d, PUp: %d, Val: %d\n", gpio_get_function(KEYS_ROW2), gpio_get_dir(KEYS_ROW2), gpio_is_pulled_down(KEYS_ROW2), gpio_is_pulled_up(KEYS_ROW2), gpio_get(KEYS_ROW2));
  printf("KEYS_ROW3: Func: %d, Dir: %d, PDown: %d, PUp: %d, Val: %d\n", gpio_get_function(KEYS_ROW3), gpio_get_dir(KEYS_ROW3), gpio_is_pulled_down(KEYS_ROW3), gpio_is_pulled_up(KEYS_ROW3), gpio_get(KEYS_ROW3));
  printf("KEYS_ROW4: Func: %d, Dir: %d, PDown: %d, PUp: %d, Val: %d\n", gpio_get_function(KEYS_ROW4), gpio_get_dir(KEYS_ROW4), gpio_is_pulled_down(KEYS_ROW4), gpio_is_pulled_up(KEYS_ROW4), gpio_get(KEYS_ROW4));
}
