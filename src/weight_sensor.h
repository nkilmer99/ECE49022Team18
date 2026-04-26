#ifndef WEIGHT_SENSOR_H
#define WEIGHT_SENSOR_H

#include "pico/async_context_freertos.h"

void HX711_init();
int32_t HX711_read_raw_adc();
void HX711_tare();
float HX711_read_kg();
void weight_worker(async_context_t *context, async_at_time_worker_t *worker);
float get_weight();

#endif
