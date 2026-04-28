#ifndef WEIGHT_SENSOR_H
#define WEIGHT_SENSOR_H

#include <stdint.h>

void HX711_init();
int32_t HX711_read_raw_adc();
void HX711_tare();
float HX711_read_kg();
void weight_worker();
float get_weight();

#endif
