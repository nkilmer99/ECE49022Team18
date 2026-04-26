#ifndef PID_TEMP_H
#define PID_TEMP_H

#include "pico/stdlib.h"
#include "stdint.h"
#include "pico/async_context_freertos.h"
#include "FreeRTOS.h"
#include "task.h"

void pid_init();
void reset_pid();
void update_pid();
void pid_worker(async_context_t *context, async_at_time_worker_t *worker);

void set_kp(float val);
float get_kp(void);

void set_ki(float val);
float get_ki(void);

void set_kd(float val);
float get_kd(void);

void set_target_temp(float val);
float get_target_temp(void);

float get_prev_error(void);
float get_integral(void);
float get_derivative(void);
float get_pid_output(void);

#endif
