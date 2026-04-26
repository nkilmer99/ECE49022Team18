#include <stdio.h>
#include <string.h>

#include "keys.h"

#include "pico/stdlib.h"

#define ROW1 3
#define ROW2 1
#define ROW3 7
#define ROW4 5
#define COL1 4
#define COL2 2
#define COL3 6

#define KEY_DELAY 5 // Poll keypad every X ms
#define BUF_SIZE 3 // Number of keys for debouncing
#define KEY_THRESH 2 // Necessary number of elements to consider "pressed"
#define KEY_SLEEP 1 // Sleep time between setting col and checking it

#define NUM_KEYS 13
const char KEYS[NUM_KEYS] = {NULL_CHAR, '1', '4', '7', '*', '2' ,'5', '8', '0', '3', '6', '9', '#'};

uint8_t key_buffer[BUF_SIZE];
uint8_t key_counts[NUM_KEYS];

bool read = false;

void keys_init() {
  // Keypad init
  printf("Init keypad\n");
  gpio_init(COL1);
  gpio_set_dir(COL1, GPIO_OUT);
  gpio_put(COL1, 0);

  gpio_init(COL2);
  gpio_set_dir(COL2, GPIO_OUT);
  gpio_put(COL2, 0);

  gpio_init(COL3);
  gpio_set_dir(COL3, GPIO_OUT);
  gpio_put(COL3, 0);

  gpio_init(ROW1);
  gpio_set_dir(ROW1, GPIO_IN);
  gpio_pull_down(ROW1);

  gpio_init(ROW2);
  gpio_set_dir(ROW2, GPIO_IN);
  gpio_pull_down(ROW2);

  gpio_init(ROW3);
  gpio_set_dir(ROW3, GPIO_IN);
  gpio_pull_down(ROW3);

  gpio_init(ROW4);
  gpio_set_dir(ROW4, GPIO_IN);
  gpio_pull_down(ROW4);

  for (int i = 0; i < BUF_SIZE; i++) {
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
  memcpy(key_buffer, &key_buffer[1], BUF_SIZE - 1); // Shift array left 1
  key_buffer[BUF_SIZE - 1] = key;

  //printf("%d %d %d %d %d\n", key_buffer[0], key_buffer[1], key_buffer[2], key_buffer[3], key_buffer[4]);
}

char get_raw_key() {
  return KEYS[key_buffer[BUF_SIZE - 1]];
}

uint8_t poll_keypad() {
  // Col 1
  gpio_put(COL1, 1);
  sleep_ms(KEY_SLEEP);
  if (gpio_get(ROW1)) {
    gpio_put(COL1, 0);
    return 1;
  } else if (gpio_get(ROW2)) {
    gpio_put(COL1, 0);
    return 2;
  } else if (gpio_get(ROW3)) {
    gpio_put(COL1, 0);
    return 3;
  } else if (gpio_get(ROW4)) {
    gpio_put(COL1, 0);
    return 4;
  }
  gpio_put(COL1, 0);

  // Col 2
  gpio_put(COL2, 1);
  sleep_ms(KEY_SLEEP);
  if (gpio_get(ROW1)) {
    gpio_put(COL2, 0);
    return 5;
  } else if (gpio_get(ROW2)) {
    gpio_put(COL2, 0);
    return 6;
  } else if (gpio_get(ROW3)) {
    gpio_put(COL2, 0);
    return 7;
  } else if (gpio_get(ROW4)) {
    gpio_put(COL2, 0);
    return 8;
  }
  gpio_put(COL2, 0);

  // Col 3
  gpio_put(COL3, 1);
  sleep_ms(KEY_SLEEP);
  if (gpio_get(ROW1)) {
    gpio_put(COL3, 0);
    return 9;
  } else if (gpio_get(ROW2)) {
    gpio_put(COL3, 0);
    return 10;
  } else if (gpio_get(ROW3)) {
    gpio_put(COL3, 0);
    return 11;
  } else if (gpio_get(ROW4)) {
    gpio_put(COL3, 0);
    return 12;
  }
  gpio_put(COL3, 0);

  return 0;
}

void debug_print_pins() {
  printf("Keypad pins:\n");
  printf("COL1: Func: %d, Dir: %d, PDown: %d, PUp: %d, Val: %d\n", gpio_get_function(COL1), gpio_get_dir(COL1), gpio_is_pulled_down(COL1), gpio_is_pulled_up(COL1), gpio_get(COL1));
  printf("COL2: Func: %d, Dir: %d, PDown: %d, PUp: %d, Val: %d\n", gpio_get_function(COL2), gpio_get_dir(COL2), gpio_is_pulled_down(COL2), gpio_is_pulled_up(COL2), gpio_get(COL2));
  printf("COL3: Func: %d, Dir: %d, PDown: %d, PUp: %d, Val: %d\n", gpio_get_function(COL3), gpio_get_dir(COL3), gpio_is_pulled_down(COL3), gpio_is_pulled_up(COL3), gpio_get(COL3));
  printf("ROW1: Func: %d, Dir: %d, PDown: %d, PUp: %d, Val: %d\n", gpio_get_function(ROW1), gpio_get_dir(ROW1), gpio_is_pulled_down(ROW1), gpio_is_pulled_up(ROW1), gpio_get(ROW1));
  printf("ROW2: Func: %d, Dir: %d, PDown: %d, PUp: %d, Val: %d\n", gpio_get_function(ROW2), gpio_get_dir(ROW2), gpio_is_pulled_down(ROW2), gpio_is_pulled_up(ROW2), gpio_get(ROW2));
  printf("ROW3: Func: %d, Dir: %d, PDown: %d, PUp: %d, Val: %d\n", gpio_get_function(ROW3), gpio_get_dir(ROW3), gpio_is_pulled_down(ROW3), gpio_is_pulled_up(ROW3), gpio_get(ROW3));
  printf("ROW4: Func: %d, Dir: %d, PDown: %d, PUp: %d, Val: %d\n", gpio_get_function(ROW4), gpio_get_dir(ROW4), gpio_is_pulled_down(ROW4), gpio_is_pulled_up(ROW4), gpio_get(ROW4));
}
