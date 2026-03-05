#include <stdio.h>

#include "ui.h"
#include "lvgl.h"
#include "ili934x.h"
#include "hardware/spi.h"
#include "ili9341.h"

// Stuff for lvgl to flush pixels to screen
ILI934X *screen = nullptr;

void test() {
  printf("Spi init\n");
  /*
  spi_init(spi0, 128 * 1000);

  printf("Running gpio_set_functions\n");
  gpio_set_function(16, GPIO_FUNC_SPI);
  gpio_set_function(18, GPIO_FUNC_SPI);
  gpio_set_function(19, GPIO_FUNC_SPI);

  printf("Init CS\n");
  gpio_init(17); // CS
  gpio_set_dir(17, GPIO_OUT);
  gpio_put(17, 1);

  printf("Init DC\n");
  gpio_init(15); // DC
  gpio_set_dir(15, GPIO_OUT);

  printf("Init RST\n");
  gpio_init(14); // RST
  gpio_set_dir(14, GPIO_OUT);
  gpio_put(14, 1);
  */

  LCD_setPins(15,17,14,18,19);
  LCD_setSPIperipf(spi0);
  LCD_initDisplay();
  LCD_WritePixel(50,50,0xF800);
  /*
  printf("New ILI934X\n");
  screen = new ILI934X(spi0, 17, 15, 14);

  printf("Screen init\n");
  screen->init();

  printf("Screen clear\n");
  while (true) {
    screen->clear(0xF800);
  }
  */
}

void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
  printf("Disp_flush!\n");
  printf("Calc width and height\n");
  int32_t width = area->x2 - area->x1 + 1;
  int32_t height = area->y2 - area->y1 + 1;

  printf("Draw to screen\n");
  //screen.blit(area->x1,area->y1,height,width,(uint16_t *)color_p);
  //screen.fillRect(0,0,320,240,0xF800);
  //screen->clear(0xF800);
  uint8_t dummy_data = 0xFF;
  spi_write_blocking(spi0, &dummy_data, 1);

  printf("Mark flush done\n");
  lv_disp_flush_ready(disp_drv);
  printf("Disp_flush done\n");
}

#define DISP_HOR_RES    320
#define DISP_VER_RES    240
#define DRAW_BUFFER_SIZE   (DISP_HOR_RES * DISP_VER_RES / 10)

static lv_color_t buf_1[DRAW_BUFFER_SIZE];
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;

void ui_init() {
  printf("Running ui_init!\n");

  printf("Running spi_init\n");
  spi_init(spi0, 30 * 1000 * 1000);

  printf("Running gpio_set_functions\n");
  gpio_set_function(16, GPIO_FUNC_SPI);
  gpio_set_function(18, GPIO_FUNC_SPI);
  gpio_set_function(19, GPIO_FUNC_SPI);

  printf("Init CS\n");
  gpio_init(17); // CS
  gpio_set_dir(17, GPIO_OUT);
  gpio_put(17, 1);

  printf("Init DC\n");
  gpio_init(15); // DC
  gpio_set_dir(15, GPIO_OUT);

  printf("Init RST\n");
  gpio_init(14); // RST
  gpio_set_dir(14, GPIO_OUT);
  gpio_put(14, 1);

  /*
  printf("Constructing object\n");
  screen = new ILI934X(spi0, 17, 15, 14);

  printf("Init screen\n");
  screen->init();
  */

  printf("Init lv\n");
  lv_init();

  printf("Init lv draw buf\n");
  lv_disp_draw_buf_init(&draw_buf, buf_1, NULL, DRAW_BUFFER_SIZE);

  // Init the display driver struct and register it
  printf("Init lv disp driver struct\n");
  lv_disp_drv_init(&disp_drv);
  disp_drv.draw_buf = &draw_buf;
  disp_drv.flush_cb = disp_flush;
  disp_drv.hor_res = DISP_HOR_RES;
  disp_drv.ver_res = DISP_VER_RES;

  printf("Register lv disp driver struct\n");
  lv_disp_drv_register(&disp_drv);

  // Test button
  printf("Make test button1\n");
  lv_obj_t * btn = lv_btn_create(lv_scr_act());
  printf("Make test button2\n");
  lv_obj_center(btn);
  printf("Make test button3\n");
  lv_obj_t * label = lv_label_create(btn);
  printf("Make test button4\n");
  lv_label_set_text(label, "Test button");
  printf("ui_init end\n");
}

void ui_worker(async_context_t *context, async_at_time_worker_t *worker) {
  printf("Worker!\n");
  printf("Worker: scheduling next worker\n");
  async_context_add_at_time_worker_in_ms(context, worker, 50); // Reschedule self for 50 ms in future

  printf("Worker: inc lv\n");
  lv_tick_inc(50);

  printf("Worker: lv_timer_handler\n");
  lv_timer_handler();

  printf("End worker\n");
}
