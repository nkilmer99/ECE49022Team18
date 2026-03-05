#include <stdio.h>

#include "ui.h"
#include "hardware/spi.h"

#define SLEEP_DELAY 1
#define ROW1 15
#define ROW2 14
#define ROW3 13
#define ROW4 12
#define COL1 11
#define COL2 10
#define COL3 9

char fail[32]  = "FAILURE         DETECTED        ";
char lines[32] = "Minutes:<      ITemp(C):        ";
#define row1_i 8
#define row2_i 24
int line_i = row1_i;
int state = 0;
char l_reg = ' ';
char l_unreg = ' ';

void ui_init() {
  printf("Spi init\n");
  spi_init(spi0, 32 * 1000);

  printf("Running gpio_set_functions\n");
  gpio_set_function(16, GPIO_FUNC_SPI);
  gpio_set_function(18, GPIO_FUNC_SPI);
  gpio_set_function(19, GPIO_FUNC_SPI);

  printf("Init CS\n");
  gpio_init(17); // CS
  gpio_set_dir(17, GPIO_OUT);
  gpio_put(17, 1);

  uint16_t display_on = 0x000F;
  uint16_t clear_display = 0x0001;
  uint16_t ret_home = 0x0002;
  uint16_t entry_set = 0x0006;
  uint16_t func_set = 0x0038;

  spi_set_format(spi0,10,0,0,SPI_MSB_FIRST);

  send_data(func_set);
  send_data(entry_set);
  send_data(ret_home);
  send_data(clear_display);
  send_data(display_on);

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

  write_lines((char *) &lines);
}

void send_data(uint16_t data) {
  gpio_put(17, 0);
  spi_write16_blocking(spi0,&data,1);
  gpio_put(17, 1);
  sleep_ms(SLEEP_DELAY);
}

void write_lines(char * lines) {
  uint16_t display_on = 0x000F;
  uint16_t clear_display = 0x0001;
  uint16_t ret_home = 0x0002;
  uint16_t entry_set = 0x0006;
  uint16_t write_add = 0x0200;
  uint16_t second_line = 0x00C0;
  uint16_t data = 0;

  send_data(ret_home);

  for (int i = 0; i < 16; i++) {
    data = write_add | (uint16_t) lines[i];
    send_data(data);
  }

  send_data(second_line);

  for (int i = 16; i < 32; i++) {
    data = write_add | (uint16_t) lines[i];
    send_data(data);
  }
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

void ui_worker(async_context_t *context, async_at_time_worker_t *worker) {
  printf("Worker!\n");
  async_context_add_at_time_worker_in_ms(context, worker, 10); // Reschedule self for 50 ms in future

  if (state == 0) {
    char key = get_key();
    printf("Key: %c\n", key);
    if (key == l_unreg && key != l_reg) {
      if (key == '#') {
        l_reg = key;
        // Failure
        for (int i = 0; i < 32; i++) {
          lines[i] = fail[i];
          state = -1;
        }
        write_lines((char *) &lines);
      } else if (key == '*') {
        lines[line_i] = ' ';
        l_reg = key;
        // Enter
        if (line_i < 16) {
          line_i = row2_i;
          lines[line_i] = '<';
        } else {
          line_i = row1_i;
          state = 1;
          lines[15] = 'A';
        }
        write_lines((char *) &lines);
      } else if (key != ' ') {
        l_reg = key;
        // Number
        lines[line_i] = key;
        line_i += 1;
        lines[line_i] = '<';
        write_lines((char *) &lines);
      } else {
        if (key == l_unreg) {
          l_reg = key;
        }
        l_unreg = key;
      }
    } else {
      l_unreg = key;
    }
  }
  else if (state != -1) {
    char key = get_key();
    if (key == '#') {
      for (int i = 0; i < 32; i++) {
        lines[i] = fail[i];
        state = -1;
      }
      write_lines((char *) &lines);
    }
    else if (key == '*' && l_reg != key) {
      l_reg = key;
      state = 0;
      lines[15] = 'I';
      for (int i = row1_i; i < 15; i++) {
        lines[i] = ' ';
      }
      for (int i = row2_i; i < 32; i++) {
        lines[i] = ' ';
      }
      line_i = row1_i;
      lines[line_i] = '<';
      write_lines((char *) &lines);
    }
    else {
      if (key == l_unreg) {
        l_reg = key;
      }
      l_unreg = key;
    }
  }

  //printf("End worker\n");
}
