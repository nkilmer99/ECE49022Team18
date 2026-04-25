#include <stdio.h>
#include <string.h>

#include "keys.h"

#include "pico/stdlib.h"

#define ROW1 3
#define ROW2 8
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
  if (key == NULL_CHAR) read = false;
  if (read) return NULL_CHAR;
  return key;
}

void mark_read() {
  read = true;
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

  printf("%c %c %c %c %c\n", key_buffer[0], key_buffer[1], key_buffer[2], key_buffer[3], key_buffer[4]);
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
