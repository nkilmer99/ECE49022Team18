#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "FreeRTOS.h"
#include "task.h"

#include "ui.h"
#include "keys.h"

#include "read_temp.h"
#include "motor_control.h"
#include "weight_sensor.h"
#include "pid_temp.h"

// Delay between led blinking
#define LED_DELAY_MS 2000

// Priorities of our threads - higher numbers are higher priority
#define MAIN_TASK_PRIORITY      ( tskIDLE_PRIORITY + 2UL )
#define BLINK_TASK_PRIORITY     ( tskIDLE_PRIORITY + 1UL )
#define WORKER_TASK_PRIORITY    ( tskIDLE_PRIORITY + 4UL )

// Stack sizes of our threads in words (4 bytes)
#define MAIN_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define BLINK_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define WORKER_TASK_STACK_SIZE 2048U

#include "pico/async_context_freertos.h"

// Turn led on or off
static void set_led(bool led_on) {
  gpio_put(PICO_DEFAULT_LED_PIN, led_on);
}

// Initialise led
static void init_led(void) {
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

void blink_task(__unused void *params) {
  bool on = false;
  init_led();
  while (true) {
    set_led(on);
    on = !on;
    vTaskDelay(LED_DELAY_MS);
  }
}

void ui_task() {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 20;

  while(1) {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    ui_worker();
  }
}

void key_task() {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 5;

  while(1) {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    key_worker();
  }
}

void temp_task() {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 1000;

  while(1) {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    temp_worker();
  }
}

void weight_task() {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 500;

  while(1) {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    weight_worker();
  }
}

void pid_task() {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 1000;

  while(1) {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    pid_worker();
  }
}

void main_task(__unused void *params) {
  DS18B20_init(); // Temp

  motor_control(OFF_MODE);

  HX711_init(); // Weight
  HX711_tare();

  pid_init();

  ui_init();

  xTaskCreate(blink_task, "BlinkThread", BLINK_TASK_STACK_SIZE, NULL, BLINK_TASK_PRIORITY, NULL);
  xTaskCreate(ui_task, "UIThread", BLINK_TASK_STACK_SIZE, NULL, BLINK_TASK_PRIORITY, NULL);
  xTaskCreate(key_task, "KeyThread", BLINK_TASK_STACK_SIZE, NULL, BLINK_TASK_PRIORITY, NULL);
  xTaskCreate(temp_task, "TempThread", BLINK_TASK_STACK_SIZE, NULL, BLINK_TASK_PRIORITY, NULL);
  xTaskCreate(weight_task, "WeightThread", BLINK_TASK_STACK_SIZE, NULL, BLINK_TASK_PRIORITY, NULL);
  xTaskCreate(pid_task, "PIDThread", BLINK_TASK_STACK_SIZE, NULL, BLINK_TASK_PRIORITY, NULL);

  while(true) {
    // Output csv to USB serial
    printf("%d,%f,%f\n", xTaskGetTickCount(), get_temp(), get_weight());
    vTaskDelay(250);
  }
}

void vLaunch(void) {
  TaskHandle_t task;

  xTaskCreate(main_task, "MainThread", MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, &task);

  // we must bind the main task to one core (well at least while the init is called)
  vTaskCoreAffinitySet(task, 1);

  /* Start the tasks and timer running. */
  vTaskStartScheduler();
}

int main(void)
{
  stdio_init_all();

  vLaunch();
  return 0;
}
