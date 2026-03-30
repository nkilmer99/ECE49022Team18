#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include "pico/stdlib.h"
#include "pico/async_context_freertos.h"
#include "read_temp.h"

void pid_init(float initial_temp);

void pid_worker(async_context_t *context, async_at_time_worker_t *worker);

void pid_set_target(float tset);

float pid_get_duty(void);

#endif
