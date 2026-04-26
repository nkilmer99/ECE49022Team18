#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>

#define WIDTH 128
#define HEIGHT 160
#define BYTES_PER_PIXEL 2

void display_init();
void reset_display();
void write_byte(bool dc, uint8_t data);
void send_command(uint8_t * data, size_t len);
void send_data(uint8_t * data, size_t len);
void write_segment(uint8_t * buffer, int x, int y, int w, int h);

#endif
