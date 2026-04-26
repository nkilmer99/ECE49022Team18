#include "pid_temp.h"

void pid_control(float input_temp)
{
    gpio_init(RELAY_PIN);
    gpio_set_dir(RELAY_PIN, GPIO_OUT);

    float integral   = 0.0f;
    float prev_error = 0.0f;
    const float dt   = SAMPLE_MS / 1000.0f;

    while (true) {
        float reading_temp = DS18B20_read_temperature();
        float error      = input_temp - reading_temp;
        integral        += error * dt;
        float derivative = (error - prev_error) / dt;
        float output     = (KP * error) + (KI * integral) + (KD * derivative);
        prev_error       = error;

       //relay on / off
        if (output > 0.0f) {
            gpio_put(RELAY_PIN, 1);
        } else {
            gpio_put(RELAY_PIN, 0);
        }

        sleep_ms(SAMPLE_MS);
    }
}
