#include <stdio.h>

#include "keys.h"

#include "pico/stdlib.h"

#define ROW1 3
#define ROW2 8
#define ROW3 7
#define ROW4 5
#define COL1 4
#define COL2 2
#define COL3 6

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
}

char get_key() {
  // Col 1
  gpio_put(COL1, 1);
  sleep_ms(1);
  if (gpio_get(ROW1)) {
    gpio_put(COL1, 0);
    return '1';
  } else if (gpio_get(ROW2)) {
    gpio_put(COL1, 0);
    return '4';
  } else if (gpio_get(ROW3)) {
    gpio_put(COL1, 0);
    return '7';
  } else if (gpio_get(ROW4)) {
    gpio_put(COL1, 0);
    return '*';
  }
  gpio_put(COL1, 0);

  // Col 2
  gpio_put(COL2, 1);
  sleep_ms(1);
  if (gpio_get(ROW1)) {
    gpio_put(COL2, 0);
    return '2';
  } else if (gpio_get(ROW2)) {
    gpio_put(COL2, 0);
    return '5';
  } else if (gpio_get(ROW3)) {
    gpio_put(COL2, 0);
    return '8';
  } else if (gpio_get(ROW4)) {
    gpio_put(COL2, 0);
    return '0';
  }
  gpio_put(COL2, 0);

  // Col 3
  gpio_put(COL3, 1);
  sleep_ms(1);
  if (gpio_get(ROW1)) {
    gpio_put(COL3, 0);
    return '3';
  } else if (gpio_get(ROW2)) {
    gpio_put(COL3, 0);
    return '6';
  } else if (gpio_get(ROW3)) {
    gpio_put(COL3, 0);
    return '9';
  } else if (gpio_get(ROW4)) {
    gpio_put(COL3, 0);
    return '#';
  }
  gpio_put(COL3, 0);

  return ' ';
}


