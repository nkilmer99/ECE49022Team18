#ifndef KEYS_H
#define KEYS_H

#include "pico/async_context_freertos.h"

#define NULL_CHAR ' '

void keys_init();
char get_key(); // Debounced key
char get_read_key(); // Debounced key, but once read it returns NULL_CHAR
void mark_read();
void key_worker(async_context_t *context, async_at_time_worker_t *worker);
char get_raw_key(); // Key as from keypad
uint8_t poll_keypad();
void debug_print_pins();

#endif
