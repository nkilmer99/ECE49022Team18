#include "pid_temp.h"

#include "read_temp.h"
#include "stdint.h"
#include "pico/async_context_freertos.h"
#include "FreeRTOS.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define PID_RELAY_PIN   22

#define PID_UPDATE_MS   1000

#define PID_I_THRESH 3

float kp = 0.0806f;   // 0.0403
float ki = 0.000045f;  // 0.00009
float kd = 4.5f;      // 4.5

float target_temp = 0.0f;

float prev_error = 0.0f;
float integral = 0.0f;
float derivative = 0.0f;
float pid_output = 0.0f;

uint32_t last_tick_count = 0;

void pid_init() {
    gpio_init(PID_RELAY_PIN);
    gpio_set_dir(PID_RELAY_PIN, GPIO_OUT);
    gpio_put(PID_RELAY_PIN, 0);
    reset_pid();
}

void reset_pid() {
  prev_error = 0.0f;
  integral = 0.0f;
  derivative = 0.0f;
  pid_output = 0.0f;
  last_tick_count = xTaskGetTickCount();
}

void update_pid() {
  float temp = get_temp();
  uint32_t tick_count = xTaskGetTickCount();

  float dt = (tick_count - last_tick_count) / 1000.0f;
  last_tick_count = tick_count;

  float error = target_temp - temp;
  bool update_int = (error < PID_I_THRESH) && (error > -PID_I_THRESH);
  if (update_int) integral += error * dt;
  derivative = (error - prev_error) / dt;
  prev_error = error;
  if (update_int) {
    pid_output = (kp * error) + (ki * integral) + (kd * derivative);
  } else {
    pid_output = (kp * error) + (kd * derivative);
  }

  if (pid_output > 0.0f) gpio_put(PID_RELAY_PIN, 1);
  else gpio_put(PID_RELAY_PIN, 0);
}

void pid_worker() {
  update_pid();
}

void set_kp(float val) { kp = val; }
float get_kp() { return kp; }

void set_ki(float val) { ki = val; }
float get_ki() { return ki; }

void set_kd(float val) { kd = val; }
float get_kd() { return kd; }

void set_target_temp(float val) { target_temp = val; }
float get_target_temp() { return target_temp; }

float get_prev_error() { return prev_error; }
float get_integral() { return integral; }
float get_derivative() { return derivative; }
float get_pid_output() { return pid_output; }
