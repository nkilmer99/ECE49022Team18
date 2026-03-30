#include "pid_controller.h"
#include "read_temp.h"
#include "pico/stdlib.h"
#include <math.h>
#include <stdio.h>


#define DT_SIM_MS    100  /* worker reschedule interval (ms)*/
#define DT_SIM_S     0.1f /* same value in seconds  */
#define DT_CTRL_S    5.0f /* PID recalculation interval (seconds)*/

#define KP           3.0f
#define KI           0.005f
#define KD           0.0f
#define BANG_ZONE    2.0f /* full-power threshold: this many °C below setpoint */

#define INTEG_MIN  -150.0f
#define INTEG_MAX   150.0f

#define TEMP_MAX     85.0f /* hard shutoff ceiling (°C) */
#define TSET_MAX     80.0f /* max user-settable temperature (°C)*/
#define SENSOR_MIN    0.0f /* sensor fault: reading below this */
#define SENSOR_MAX  100.0f /* sensor fault: reading above this*/

static float s_tset             = 65.0f; /* current setpoint, default 65°C */
static float s_duty             = 0.0f; /* latest duty cycle output*/
static float s_integ            = 0.0f;
static float s_prev_meas        = 0.0f;
static float s_duty_hold        = 0.0f;
static int   s_count            = 0;
static bool  s_initialized      = false;
static int   s_steps_per_update = 0;

void pid_init(float initial_temp)
{
    s_tset             = 65.0f;
    s_duty             = 0.0f;
    s_integ            = 0.0f;
    s_prev_meas        = initial_temp;
    s_duty_hold        = 0.0f;
    s_count            = 0;
    s_initialized      = true;
    s_steps_per_update = (int)(DT_CTRL_S / DT_SIM_S);
    if (s_steps_per_update < 1) s_steps_per_update = 1;

    printf("[PID] init  Tset=%.1f  steps_per_update=%d\n",
           s_tset, s_steps_per_update);
}

void pid_set_target(float tset)
{
    if (tset > TSET_MAX) {
        printf("[PID SAFETY] Setpoint %.1f clamped to %.1f\n", tset, TSET_MAX);
        tset = TSET_MAX;
    }
    if (tset < 0.0f) tset = 0.0f;

    s_tset = tset;
    printf("[PID] new setpoint: %.1f C\n", s_tset);
}

float pid_get_duty(void)
{
    return s_duty;
}

void pid_worker(async_context_t *context, async_at_time_worker_t *worker)
{
    async_context_add_at_time_worker_in_ms(context, worker, DT_SIM_MS);

    if (!s_initialized) return;

    s_count++;

    if (s_count >= s_steps_per_update)
    {
        s_count = 0;

        float Tmeas = DS18B20_read_temperature();

        /* SAFETY CHECK 1*/
        if (Tmeas < SENSOR_MIN || Tmeas > SENSOR_MAX) {
            printf("[PID SAFETY] Sensor fault: %.2f out of range. Heater OFF.\n", Tmeas);
            s_duty      = 0.0f;
            s_duty_hold = 0.0f;
            return;
        }

        /* SAFETY CHECK 2*/
        if (Tmeas >= TEMP_MAX) {
            printf("[PID SAFETY] Over-temp: %.2f >= %.1f. Heater OFF.\n",
                   Tmeas, TEMP_MAX);
            s_integ     = INTEG_MIN;   /* drain integrator — prevents immediate re-fire */
            s_duty      = 0.0f;
            s_duty_hold = 0.0f;
            return;
        }

        /* PID calculation */
        float err   = s_tset - Tmeas;
        float dMeas = (Tmeas - s_prev_meas) / DT_CTRL_S;
        s_prev_meas = Tmeas;

        float duty;

        if (err > BANG_ZONE)
        {
            /* Bang-bang: far below setpoint, then full power
             * Integrator NOT pre-loaded. prevents overshoot on transition */
            duty = 1.0f;
        }
        else
        {
            /* Normal PID */
            s_integ += err * DT_CTRL_S;
            if (s_integ < INTEG_MIN) s_integ = INTEG_MIN;
            if (s_integ > INTEG_MAX) s_integ = INTEG_MAX;

            float u = KP * err + KI * s_integ - KD * dMeas;
            duty = u / 100.0f;
            if (duty < 0.0f) duty = 0.0f;
            if (duty > 1.0f) duty = 1.0f;
        }

        s_duty_hold = duty;
        s_duty      = duty;

        printf("[PID] Tset=%.1f  Tmeas=%.2f  err=%.2f  duty=%.3f\n",
               s_tset, Tmeas, err, s_duty);
    }
    else
    {
        /* Hold duty between recalculation*/
        s_duty = s_duty_hold;
    }
}
