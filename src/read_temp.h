#ifndef READ_TEMP_H
#define READ_TEMP_H

#include "pico/stdlib.h"
#include "pico/async_context_freertos.h"
#include "FreeRTOS.h"
#include "task.h"

int DS18B20_reset();
void onewire_write_bit(bool bit);
int onewire_read_bit();
void onewire_write_byte(uint8_t byte);
uint8_t onewire_read_byte();
void DS18B20_init();
float DS18B20_read_temperature();
void temp_tester();
void temp_worker(async_context_t *context, async_at_time_worker_t *worker);

#endif