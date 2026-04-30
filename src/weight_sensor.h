#ifndef WEIGHT_SENSOR_H
#define WEIGHT_SENSOR_H

#include <stdint.h>

enum {
  WEIGHT_BEEF = 0,
  WEIGHT_CHICKEN,
  WEIGHT_PORK
};

void HX711_init();
int32_t HX711_read_raw_adc();
void HX711_tare();
float HX711_read_kg();
void weight_worker();
float get_weight();
uint32_t get_time_from_weight(int food);

#endif
