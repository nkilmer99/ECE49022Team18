#include <stdio.h>

#include "ui.h"
#include "hardware/spi.h"

#define SPI_DELAY 1
#define ROW1 3
#define ROW2 8
#define ROW3 7
#define ROW4 5
#define COL1 4
#define COL2 2
#define COL3 6

#define CS   17
#define MOSI 19
#define MISO 16
#define SCLK 18
#define DC   8

void ui_init() {
  printf("Spi init\n");
  spi_init(spi0, 128 * 1024);

  gpio_set_function(CS, GPIO_FUNC_SPI);
  gpio_set_function(SCLK, GPIO_FUNC_SPI);
  gpio_set_function(MOSI, GPIO_FUNC_SPI);

  gpio_init(CS);
  gpio_set_dir(CS, GPIO_OUT);
  gpio_put(CS, 1);

  gpio_init(DC);
  gpio_set_dir(DC, GPIO_OUT);
  gpio_put(DC, 0);

  spi_set_format(spi0,12,0,0,SPI_MSB_FIRST); // 12 data bits

  write_byte(0, 0x11);
  write_byte(0, 0xB1);
  write_byte(1, 0x05);
  write_byte(1, 0x3A);
  write_byte(1, 0x3A);

  write_byte(0, 0xB2);
  write_byte(1, 0x05);
  write_byte(1, 0x3A);
  write_byte(1, 0x3A);

  write_byte(0, 0xB3);
  write_byte(1, 0x05);
  write_byte(1, 0x3A);
  write_byte(1, 0x3A);
  write_byte(1, 0x05);
  write_byte(1, 0x3A);
  write_byte(1, 0x3A);

  write_byte(0, 0xB4);
  write_byte(1, 0x03);

  write_byte(0, 0xC0);
  write_byte(1, 0x62);
  write_byte(1, 0x02);
  write_byte(1, 0x04);

  write_byte(0, 0xC1);
  write_byte(1, 0xC0);

  write_byte(0, 0xC2);
  write_byte(1, 0x0D);
  write_byte(1, 0x00);

  write_byte(0, 0xC3);
  write_byte(1, 0x8D);
  write_byte(1, 0x6A);

  write_byte(0, 0xC4);
  write_byte(1, 0x8D);
  write_byte(1, 0xEE);

  write_byte(0, 0xC5);
  write_byte(1, 0x0E);

  write_byte(0, 0xE0);
  write_byte(1, 0x10);
  write_byte(1, 0x0E);
  write_byte(1, 0x02);
  write_byte(1, 0x03);
  write_byte(1, 0x0E);
  write_byte(1, 0x07);
  write_byte(1, 0x02);
  write_byte(1, 0x07);
  write_byte(1, 0x0A);
  write_byte(1, 0x12);
  write_byte(1, 0x27);
  write_byte(1, 0x37);
  write_byte(1, 0x00);
  write_byte(1, 0x0D);
  write_byte(1, 0x0E);
  write_byte(1, 0x10);

  write_byte(0, 0xE1);
  write_byte(1, 0x10);
  write_byte(1, 0x0E);
  write_byte(1, 0x03);
  write_byte(1, 0x03);
  write_byte(1, 0x0F);
  write_byte(1, 0x06);
  write_byte(1, 0x02);
  write_byte(1, 0x08);
  write_byte(1, 0x0A);
  write_byte(1, 0x13);
  write_byte(1, 0x26);
  write_byte(1, 0x36);
  write_byte(1, 0x00);
  write_byte(1, 0x0D);
  write_byte(1, 0x0E);
  write_byte(1, 0x10);

  write_byte(0, 0x3A);
  write_byte(1, 0x05);

  write_byte(0, 0x29);

  write_red();

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

void write_byte(bool dc, uint8_t data) {
  gpio_put(DC, dc);
  gpio_put(CS, 0);

  spi_write_blocking(spi0, &data, 1);

  gpio_put(DC, 0);
  gpio_put(CS, 1);

  sleep_ms(SPI_DELAY);
}

void send_data(bool dc, uint16_t * data, size_t len) {
  gpio_put(DC, dc);
  gpio_put(CS, 0);

  spi_write16_blocking(spi0, data, len);

  gpio_put(DC, 0);
  gpio_put(CS, 1);

  sleep_ms(SPI_DELAY);
}

void write_red() {
  printf("Writing red!\n");
  gpio_put(DC, 1);

  uint16_t data = 0x0F0F;

  for (int i = 0; i < 20480; i++) {
    gpio_put(CS, 0);
    spi_write16_blocking(spi0, &data, 1);
    gpio_put(CS, 1);
  }

  gpio_put(DC, 0);
  printf("Done writing red!\n");
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
  char key = get_key();
  printf("Key: %c\n", key);
  printf("End Worker!\n");
  /*
  if (state == 0) {
    char key = get_key();
    //printf("Key: %c\n", key);
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
          motor_control(CONTROLLED_MODE);
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
        motor_control(OFF_MODE);
      }
      write_lines((char *) &lines);
    }
    else if (key == '*' && l_reg != key) {
      l_reg = key;
      state = 0;
      motor_control(OFF_MODE);
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
  */
}
