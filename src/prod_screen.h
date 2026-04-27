#ifndef PROD_SCREEN_H
#define PROD_SCREEN_H

void prod_screen_init();
void prod_set_active();
void all_off();
void switch_screen(int screen);

void init_n_button(uint32_t screen, uint32_t n_buttons);
void init_n_label(uint32_t screen, uint32_t n_labels);
void init_n_input(uint32_t screen, uint32_t n_inputs);

void prod_update_screen(char key);
void update_prod_screen_init(char key);
void update_prod_screen_preset(char key);
void update_prod_screen_custom(char key);
void update_prod_screen_warmup(char key);
void update_prod_screen_cooking(char key);
void update_prod_screen_error(char key);
void update_prod_screen_done(char key);

#endif
