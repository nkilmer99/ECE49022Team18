#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "FreeRTOS.h"
#include "task.h"

#include "ui.h"

// Delay between led blinking
#define LED_DELAY_MS 2000

// Priorities of our threads - higher numbers are higher priority
#define MAIN_TASK_PRIORITY      ( tskIDLE_PRIORITY + 2UL )
#define BLINK_TASK_PRIORITY     ( tskIDLE_PRIORITY + 1UL )
#define WORKER_TASK_PRIORITY    ( tskIDLE_PRIORITY + 4UL )

// Stack sizes of our threads in words (4 bytes)
#define MAIN_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define BLINK_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define WORKER_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

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
  printf("blink_task starts\n");
  init_led();
  while (true) {
    static int last_core_id = -1;
    if (portGET_CORE_ID() != last_core_id) {
      last_core_id = portGET_CORE_ID();
      printf("blink task is on core %d\n", last_core_id);
    }
    set_led(on);
    on = !on;

    sleep_ms(LED_DELAY_MS);
  }
}

static async_context_freertos_t async_context_instance;

// Create an async context
static async_context_t *create_async_context(void) {
  async_context_freertos_config_t config = async_context_freertos_default_config();
  config.task_priority = WORKER_TASK_PRIORITY; // defaults to ASYNC_CONTEXT_DEFAULT_FREERTOS_TASK_PRIORITY
  config.task_stack_size = WORKER_TASK_STACK_SIZE; // defaults to ASYNC_CONTEXT_DEFAULT_FREERTOS_TASK_STACK_SIZE
  if (!async_context_freertos_init(&async_context_instance, &config))
    return NULL;
  return &async_context_instance.core;
}

async_at_time_worker_t ui_timeout = { .do_work = ui_worker };

void main_task(__unused void *params) {
  printf("Start main task\n");
  async_context_t *context = create_async_context();

  printf("Running ui_init\n");
  ui_init();

  // start the worker running
  printf("Starting worker\n");
  async_context_add_at_time_worker_in_ms(context, &ui_timeout, 0);

  // start the led blinking
  printf("Starting led\n");
  xTaskCreate(blink_task, "BlinkThread", BLINK_TASK_STACK_SIZE, NULL, BLINK_TASK_PRIORITY, NULL);

  printf("Starting main loop\n");
  int count = 0;
  while(true) {
    static int last_core_id = -1;
    if (portGET_CORE_ID() != last_core_id) {
      last_core_id = portGET_CORE_ID();
      printf("main task is on core %d\n", last_core_id);
    }
    printf("Hello from main task count=%u\n", count++);
    vTaskDelay(3000);
  }
  async_context_deinit(context);
}

void vLaunch( void) {
  TaskHandle_t task;

  xTaskCreate(main_task, "MainThread", MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, &task);

  // we must bind the main task to one core (well at least while the init is called)
  vTaskCoreAffinitySet(task, 1);

  /* Start the tasks and timer running. */
  vTaskStartScheduler();
}

int main( void )
{
  stdio_init_all();
  printf("Start1\n");
  sleep_ms(2000);
  printf("Start2\n");
  sleep_ms(2000);
  printf("Start3\n");
  sleep_ms(2000);
  printf("Start4\n");
  sleep_ms(2000);

  vLaunch();
  return 0;
}
