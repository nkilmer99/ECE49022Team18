#include <stdio.h>

#include "weight_sensor.h"

#include "pico/stdlib.h"

#define WEIGHT_DATA_PIN 24 // Data Pin for HX711
#define WEIGHT_SCK_PIN 23 // Clock Pin for HX711
#define TARE_SAMPLES 20 // Times read for Taring
#define KG_SAMPLES 10 // Times read for kg reading

#define WEIGHT_CALIBRATION_VALUE 91760.0f // Calibration Value
/*
PROCEDURE TO FIND CALIBRATION VALUE

1. Tare the scale using HX711_tare(); this finds your OFFSET

2. Place Known weight (1kg)

3. Read raw adc value using HX711_read_raw_adc();
Subtract offset from raw value, call it X.
X = raw - offset

4. your calibration value is X/(known weight). So if it is
1kg, then WEIGHT_CALIBRATION_VALUE = X/1kg = X. Set this in the define
statement
*/

int32_t OFFSET = 0; // Initialize global OFFSET variable

// Initialize Pins for SCK and DATA
void HX711_init()
{
    gpio_init(WEIGHT_DATA_PIN); // Initialize Data Pin
    gpio_set_dir(WEIGHT_DATA_PIN, GPIO_IN);
    gpio_pull_up(WEIGHT_DATA_PIN);

    gpio_init(WEIGHT_SCK_PIN); // SCK Pin
    gpio_set_dir(WEIGHT_SCK_PIN, GPIO_OUT);
    gpio_put(WEIGHT_SCK_PIN, 0);
}

// This Function reads raw ADC values, sets gain to 128 on 25th clock pulse.
// Use this to find calibration value
int32_t HX711_read_raw_adc()
{
    while(gpio_get(WEIGHT_DATA_PIN) == 1); // Wait for Ready

    int32_t raw_value = 0; // initialize raw ADC value variable

    for (int i = 0; i < 24; i++) // Loop 24 times for 24-bit ADC
    {
        gpio_put(WEIGHT_SCK_PIN, 1); // Starts Clock
        sleep_us(1);

        raw_value = (raw_value << 1) | gpio_get(WEIGHT_DATA_PIN); // Reads bit and shifts value

        gpio_put(WEIGHT_SCK_PIN, 0); // Ends Clock
        sleep_us(1);
    }

    // Pulse 25 on clock sets gain 128 Channel A
    gpio_put(WEIGHT_SCK_PIN, 1);
    sleep_us(1);
    gpio_put(WEIGHT_SCK_PIN, 0);
    sleep_us(1);

    // This Extends 24 bit to int32_t via |=
    if (raw_value & 0x800000)
    {
      raw_value |= 0xFF000000;
    }

    return raw_value;
}

// Function to Tare, defines OFFSET variable
void HX711_tare()
{
    int64_t sum_raw_readings = 0; // Sum of raw readings variable
    int32_t avg_offset; // average offset variable from multiple readings

    for (int i = 0; i < TARE_SAMPLES; i++) // Read 20 times to tare
    {
        sum_raw_readings += HX711_read_raw_adc();
    }

    avg_offset = sum_raw_readings / TARE_SAMPLES; // Average Offset from 20 readings (limits noise)

    OFFSET = avg_offset; // set global OFFSET to average
}

// Function to return kg reading
float HX711_read_kg()
{
    int64_t sum_raw_readings = 0; // Sum of raw readings variable
    int32_t avg_raw_reading; // average raw readings variable from multiple readings
    float kg_reading; // kg reading stored in float

    for (int i = 0; i < KG_SAMPLES; i++) // Read 10 times to read kg
    {
        sum_raw_readings += HX711_read_raw_adc();
    }

    avg_raw_reading = sum_raw_readings / KG_SAMPLES; // average readings

    kg_reading = (avg_raw_reading - OFFSET) / WEIGHT_CALIBRATION_VALUE; // equation to calculate kg

    return kg_reading;
}

// Tare for 10 second timer
void tare_10s_tester()
{
    // Taring for 10 Seconds to Zero Scale
    for (int i = 0; i < 1; i++)
    {
        printf("Taring %d...\n", i + 1);
        sleep_ms(1000);
    }
    HX711_tare();
}

// Read kg or raw
void read_tester()
{
    float weight = HX711_read_kg();
    printf("Weight: %.2f kg\n", weight);
    //int32_t raw = HX711_read_raw_adc(); //Uncomment for Calibration
    //printf("Raw: %ld\n", raw);
}


void weight_worker(async_context_t *context, async_at_time_worker_t *worker) {
  async_context_add_at_time_worker_in_ms(context, worker, 500); // Reschedule self for x ms in the future

  read_tester();
}
