#include "prod_screen.h"

#include "lvgl.h"

lv_obj_t * prod_screen = NULL;

void prod_screen_init() {
  // Init style
  lv_style_t prod_style;
  lv_style_init(&prod_style);
  lv_style_set_radius(&prod_style, 5);

  lv_style_set_width(&prod_style, 128);
  lv_style_set_height(&prod_style, 160);

  lv_style_set_pad_ver(&prod_style, 10);
  lv_style_set_pad_hor(&prod_style, 10);

  lv_style_set_x(&prod_style, 0);
  lv_style_set_y(&prod_style, 0);

  lv_style_set_bg_opa(&prod_style, LV_OPA_COVER);

  static lv_grad_dsc_t grad;
  grad.dir = LV_GRAD_DIR_VER;
  grad.stops_count = 2;
  grad.stops[0].color = lv_palette_lighten(LV_PALETTE_GREY, 1);
  grad.stops[0].opa = LV_OPA_COVER;
  grad.stops[1].color = lv_palette_main(LV_PALETTE_BLUE);
  grad.stops[1].opa = LV_OPA_COVER;

  /*Shift the gradient to the bottom*/
  grad.stops[0].frac  = 80;
  grad.stops[1].frac  = 160;

  lv_style_set_bg_grad(&prod_style, &grad);

  // Make object
  prod_screen = lv_obj_create(NULL);
  lv_obj_add_style(prod_screen, &prod_style, 0);

  // Make label
  lv_obj_t * label = lv_label_create(prod_screen);
  lv_label_set_text(label, "Prod");
}

void prod_set_active() {
  lv_screen_load(prod_screen);
}

void prod_update_screen(char key) {

}
