#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "pico/stdlib.h"

void motor_control(int mode);
int get_mode();

#define MAX_MODE 2
#define CONTROLLED_MODE 1
#define OFF_MODE 0

#endif
