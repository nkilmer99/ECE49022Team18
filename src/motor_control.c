#include "motor_control.h"
#include "hardware/pwm.h"

#define PWM_PIN 21

#define MAX_MODE 2
#define CONTROLLED_MODE 1
#define OFF_MODE 0

void motor_control(int mode)
{
    // Set PWM_PIN to output PWM
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);

    // PWM slice and channel
    uint slice = pwm_gpio_to_slice_num(PWM_PIN);
    uint channel = pwm_gpio_to_channel(PWM_PIN);

    // Set counter rap value to 999
    pwm_set_wrap(slice, 999);

    // Clock Divider for frequency (12kHz)
    pwm_set_clkdiv(slice, 12.5f);  

    // 0% DC initially (motor off)
    pwm_set_chan_level(slice, channel, 0);

    // Enable PWM
    pwm_set_enabled(slice, true);

    // Logic to Choose Mode
    if (mode == MAX_MODE)
    {
        pwm_set_chan_level(slice, channel, 1000); // 100% DC
    }
    else if (mode == CONTROLLED_MODE)
    {
        uint16_t duty = (999 + 1) * 0.30f;
        pwm_set_chan_level(slice, channel, duty); // 30% DC
    }
    else
    {
        pwm_set_chan_level(slice, channel, 0); // PWM OFF
    }
}