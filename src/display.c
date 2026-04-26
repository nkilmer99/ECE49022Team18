#include <stdio.h>

#include "display.h"

#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "FreeRTOS.h"

#define DISPLAY_RST_DELAY 100 // ms
#define DISPLAY_SPI_DELAY 0
#define DISPLAY_CS   17
#define DISPLAY_MOSI 19
#define DISPLAY_MISO 16
#define DISPLAY_SCLK 18
#define DISPLAY_DC   10
#define DISPLAY_BLK  9
#define DISPLAY_RST  11

// Reset display
void reset_display() {
  gpio_put(DISPLAY_RST, 0);
  sleep_ms(DISPLAY_RST_DELAY);
  gpio_put(DISPLAY_RST, 1);
  sleep_ms(DISPLAY_RST_DELAY);
}

// Send byte to display
void write_byte(bool dc, uint8_t data) {
  gpio_put(DISPLAY_DC, dc);
  gpio_put(DISPLAY_CS, 0);

  spi_write_blocking(spi0, &data, 1);

  gpio_put(DISPLAY_DC, 1);
  gpio_put(DISPLAY_CS, 1);

  sleep_ms(DISPLAY_SPI_DELAY);
}

// Send command to display
void send_command(uint8_t * data, size_t len) {
  gpio_put(DISPLAY_DC, 0);
  gpio_put(DISPLAY_CS, 0);

  spi_write_blocking(spi0, data, 1);

  gpio_put(DISPLAY_DC, 1);

  if (len > 1) {
    spi_write_blocking(spi0, &data[1], len - 1);
  }

  gpio_put(DISPLAY_CS, 1);

  sleep_ms(DISPLAY_SPI_DELAY);
}

// Send data to display
void send_data(uint8_t * data, size_t len) {
  gpio_put(DISPLAY_DC, 1);
  gpio_put(DISPLAY_CS, 0);

  spi_write_blocking(spi0, data, len);

  gpio_put(DISPLAY_DC, 1);
  gpio_put(DISPLAY_CS, 1);

  sleep_ms(DISPLAY_SPI_DELAY);
}

// Write rectangular buffer to region on display
void write_segment(uint8_t * buffer, int x, int y, int w, int h) {
  uint8_t xlimits[5] = {0x2A, 0x00, (uint8_t) (x + 2), 0x00, (uint8_t) (x + w + 1)};
  send_command(xlimits, 5);

  uint8_t ylimits[5] = {0x2B, 0x00, (uint8_t) (y + 1), 0x00, (uint8_t) (y + h)};
  send_command(ylimits, 5);

  write_byte(0, 0x2C);
  send_data(buffer, 2 * w * h);
}

// Configure display
void display_init() {
  printf("Spi init\n");
  spi_init(spi0, 60 * 1000 * 1000);

  gpio_set_function(DISPLAY_SCLK, GPIO_FUNC_SPI);
  gpio_set_function(DISPLAY_MOSI, GPIO_FUNC_SPI);
  gpio_set_function(DISPLAY_MISO, GPIO_FUNC_SPI);

  spi_set_format(spi0,8,0,0,SPI_MSB_FIRST); // 8 data bits

  gpio_init(DISPLAY_CS);
  gpio_set_dir(DISPLAY_CS, GPIO_OUT);
  gpio_put(DISPLAY_CS, 1);

  gpio_init(DISPLAY_DC);
  gpio_set_dir(DISPLAY_DC, GPIO_OUT);
  gpio_put(DISPLAY_DC, 0);

  gpio_init(DISPLAY_BLK);
  gpio_set_dir(DISPLAY_BLK, GPIO_OUT);
  gpio_put(DISPLAY_BLK, 0);

  gpio_init(DISPLAY_RST);
  gpio_set_dir(DISPLAY_RST, GPIO_OUT);
  gpio_put(DISPLAY_RST, 1);
  reset_display();
  gpio_put(DISPLAY_BLK, 1);

  // DC = 0 when command, 1 when data
  // RST is active low
  // CS is active low
  // 3 = DC
  // 4 = CS
  // 5 = RST

  // Bytes captured from ref impl:
  // 11 Sleep out & booster on
  //
  // B1 In normal mode (Full colors)
  // 05
  // 3C
  // 3C
  //
  // B2 In idle mode (8 colors)
  // 05
  // 3C
  // 3C
  //
  // B3 In partial mode + full colors
  // 05
  // 3C
  // 3C
  // 05
  // 3C
  // 3C
  //
  // B4 Display inversion control
  // 03
  //
  // C0 Power control setting
  // 28
  // 08
  // 04
  //
  // C1 Power control setting
  // C0
  //
  // C2 In normal mode (Full colors)
  // 0D
  // 00
  //
  // C3 In idle mode (8-colors)
  // 8D
  // 2A
  //
  // C4 In partial mode + Full colors
  // 8D
  // EE
  //
  // C5 VCOM control 1
  // 1A
  //
  // 3A Interface pixel format
  // 05
  //
  // 36 Memory data access control
  // C0
  //
  // E0 Set gamma adjustment
  // 04
  // 22
  // 07
  // 0A
  // 2E
  // 30
  // 25
  // 2A
  // 28
  // 26
  // 2E
  // 3A
  // 00
  // 01
  // 03
  // 13
  //
  // E1 Set gamma adjustment
  // 04
  // 16
  // 06
  // 0D
  // 2D
  // 26
  // 23
  // 27
  // 27
  // 25
  // 2D
  // 3B
  // 00
  // 01
  // 04
  // 13
  //
  // 29 Display on
  //
  // 36 Memory data access control
  // 05
  //
  // 36 Memory data access control
  // 65
  //
  // 2A Column address set
  // 00
  // 01
  // 00
  // A0
  //
  // 2B Row address set
  // 00
  // 02
  // 00
  // 81
  //
  // 2C Memory write
  // 00
  // (Pixel data ...)

  printf("Configuring display!\n");

  // Sleep out & booster on
  write_byte(0, 0x11);

  // Framerate control
  // In normal mode (Full colors)
  uint8_t w_data1[4] = {0xB1, 0x05, 0x3C, 0x3C};
  send_command(w_data1, 4);

  // In idle mode (8 colors)
  uint8_t w_data2[4] = {0xB2, 0x05, 0x3C, 0x3C};
  send_command(w_data2, 4);

  // In partial mode + full colors
  uint8_t w_data3[7] = {0xB3, 0x05, 0x3C, 0x3C, 0x05, 0x3C, 0x3C};
  send_command(w_data3, 7);

  // Display inversion control
  uint8_t w_data4[2] = {0xB4, 0x03};
  send_command(w_data4, 2);

  // Power control setting
  uint8_t w_data5[4] = {0xC0, 0x28, 0x08, 0x04};
  send_command(w_data5, 4);

  // Power control setting
  uint8_t w_data6[2] = {0xC1, 0xC0};
  send_command(w_data6, 2);

  // In normal mode (Full colors)
  uint8_t w_data7[3] = {0xC2, 0x0D, 0x00};
  send_command(w_data7, 3);

  // In idle mode (8 colors)
  uint8_t w_data8[3] = {0xC3, 0x8D, 0x2A};
  send_command(w_data8, 3);

  // In partial mode + Full colors
  uint8_t w_data9[3] = {0xC4, 0x8D, 0xEE};
  send_command(w_data9, 3);

  // VCOM control 1
  uint8_t w_data10[2] = {0xC5, 0x1A};
  send_command(w_data10, 2);

  // Interface pixel format
  // 0x03 = 12-bit/pixel (4R, 4G, 4B)
  // 0x05 = 16-bit/pixel (5R, 6G, 5B)
  // 0x06 = 18-bit/pixel (6R 2-, 6G 2-, 6B 2-)
  uint8_t w_data11[2] = {0x3A, 0x05};
  send_command(w_data11, 2);

  // Set gamma adjustment
  uint8_t w_data13[17] = {0xE0, 0x04, 0x22, 0x07, 0x0A, 0x2E, 0x30, 0x25, 0x2A, 0x28, 0x26, 0x2E, 0x3A, 0x00, 0x01, 0x03, 0x13};
  send_command(w_data13, 17);

  // Set gamma adjustment
  uint8_t w_data14[17] = {0xE1, 0x04, 0x16, 0x06, 0x0D, 0x2D, 0x26, 0x23, 0x27, 0x27, 0x25, 0x2D, 0x3B, 0x00, 0x01, 0x04, 0x13};
  send_command(w_data14, 17);

  // Display on
  write_byte(0, 0x29);

  // Memory data access control
  // MY, MX, MV, ML, RGB, MH, -, -
  // MY = Row Address Order
  // MX = Column Address Order
  // MV = Row/Column Exchange
  // ML = Vertical Refresh Order
  // RGB = RGB-BGR order (0 = RGB)
  // MH = Horizontal Refresh Order
  uint8_t w_data16[2] = {0x36, 0x00};
  send_command(w_data16, 2);

  // Display is from (inclusive):
  // XS = 0x02 to XE = 0x81
  // YS = 0x01 to YE = 0xA0
  // Column address set {XS1, XS0, XE1, XE0}
  uint8_t w_data17[5] = {0x2A, 0x00, 0x02, 0x00, 0x81};
  send_command(w_data17, 5);

  // Row address set {YS1, YS0, YE1, YE0}
  uint8_t w_data18[5] = {0x2B, 0x00, 0x01, 0x00, 0xA0};
  send_command(w_data18, 5);
  printf("Display configured!\n");

  // Test write
  printf("Writing to display!\n");

  // Display buffer
  // 2 bytes per pixel, 128 x 160 = 20480 pixels, 40960 bytes
  uint8_t * display_buffer = pvPortMalloc(sizeof(uint8_t) * 40960);
  for (int i = 0; i < 20480; i++) {
    display_buffer[2 * i] = 0xF8;
    display_buffer[(2 * i) + 1] = 0x00;
  }

  // Write buffer to display
  write_segment(display_buffer, 0, 0, 128, 160);

  // Write blue to middle of display
  for (int i = 0; i < 20480; i++) {
    display_buffer[2 * i] = 0x00;
    display_buffer[(2 * i) + 1] = 0x1F;
  }
  write_segment(display_buffer, 42, 40, 44, 80);

  vPortFree(display_buffer);
}
