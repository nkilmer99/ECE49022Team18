#ifndef PID_TEMP_H
#define PID_TEMP_H

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "read_temp.h"

#define RELAY_PIN   22

#define KP  0.0403f
#define KI  0.00009f
#define KD  4.5f

#define SAMPLE_MS   1000

void pid_control(float input_temp);

#endif
