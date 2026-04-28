#include <stdio.h>
#include "pico/stdlib.h"
#include "read_temp.h"

#define TEMPSENSOR_PIN 21 // Set Temp Sensor Pin (Set in header file as well)
#define TEMP_ERROR -999.0f // Default TEMP_ERROR Number for TEMP_ERROR cases

/*
This code reads temperature in Celsius using 1 DS18B20 Sensor.
It reads at highest resolution, only allowing 1 reading every
750ms. It can be tuned differently.
*/

/*
Initialization consisting of Reset Pulse sent by
mcu, followed by presence pulse from slave. See figure 15
on datasheet for DS18B20.
*/
int DS18B20_reset()
{
    int presence_pulse;

    gpio_set_dir(TEMPSENSOR_PIN, GPIO_OUT); // set GPIO to output
    gpio_put(TEMPSENSOR_PIN, 0); // Pull low
    sleep_us(480); // minimum 480us for reset pulse

    gpio_set_dir(TEMPSENSOR_PIN, GPIO_IN); // set GPIO to input (External resistor pulls line high)
    sleep_us(70); // DS18B20 waits 15-60us according to datasheet to pull line low for reading

    presence_pulse = !gpio_get(TEMPSENSOR_PIN); // reading presence pulse, invert since pulled low
    sleep_us(410); // Master 480us minimum, so 410 + 70 = 480

    return presence_pulse;
}

// Write Bit
void onewire_write_bit(bool bit)
{
    gpio_set_dir(TEMPSENSOR_PIN, GPIO_OUT); // set GPIO output
    gpio_put(TEMPSENSOR_PIN, 0); // Start by pulling line low

    if (bit) // Within spec of 60-120us by using 70us
    {
        // Write 1
        sleep_us(10);
        gpio_set_dir(TEMPSENSOR_PIN, GPIO_IN); // release line
        sleep_us(60);
    }
    else
    {
        // Write 0
        sleep_us(60);
        gpio_set_dir(TEMPSENSOR_PIN, GPIO_IN); // release line
        sleep_us(10);
    }
}

// Read Bit
int onewire_read_bit()
{
    int bit; // bit variable for reading

    gpio_set_dir(TEMPSENSOR_PIN, GPIO_OUT); // set GPIO to output
    gpio_put(TEMPSENSOR_PIN, 0); // Pull Line low
    sleep_us(2); // Within 1-15us

    gpio_set_dir(TEMPSENSOR_PIN, GPIO_IN); // release line
    sleep_us(13); // wait enough time (2 + 13 = 15us)

    bit = gpio_get(TEMPSENSOR_PIN);
    sleep_us(55); // Add 55 us so it totals 70 us like write bit

    return bit;
}

// Write Byte
void onewire_write_byte(uint8_t byte)
{
    for (int i = 0; i < 8; i++)
    {
        onewire_write_bit(byte & 0x01);
        byte >>= 1;
    }
}

// Read Byte
uint8_t onewire_read_byte()
{
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++)
    {
        byte >>= 1;
        if (onewire_read_bit()) byte |= 0x80;
    }
    return byte;
}

// Initialize Pins
void DS18B20_init()
{
    gpio_init(TEMPSENSOR_PIN); // Initialize Temp Sensor Pin
    gpio_put(TEMPSENSOR_PIN, 1);
}

// Read Temperature and return float in degrees C.
float DS18B20_read_temperature()
{
    int presence_signal;

    presence_signal = DS18B20_reset(); // Send Reset

    if (presence_signal == 0)
    {
        //printf("<Sensor Undetected>\n"); // Serial Print Undetected Sensor
        return TEMP_ERROR;
    }

    onewire_write_byte(0xCC); // Skip ROM on datasheet command
    onewire_write_byte(0x44); // Convert T on datasheet command

    printf("Temp delay\n");
    vTaskDelay(750); // Required for 12-bit resolution (default) on sensor
    printf("Temp end delay\n");

    presence_signal = DS18B20_reset(); // Send Reset

    if (presence_signal == 0)
    {
        //printf("<Sensor Undetected>\n"); // Serial Print Undetected Sensor
        return TEMP_ERROR;
    }

    onewire_write_byte(0xCC); // Skip ROM
    onewire_write_byte(0xBE); // Read Scratchpad on datasheet command

    uint8_t LSB = onewire_read_byte();
    uint8_t MSB = onewire_read_byte();
    int16_t temp = (MSB << 8) | LSB; // Store Bits into temp

    float temp_c = temp / 16.0f; // Convert to celsius
    return temp_c;
}

float last_temperature = 0.0;

void temp_worker(async_context_t *context, async_at_time_worker_t *worker)
{
    async_context_add_at_time_worker_in_ms(context, worker, 1000); // Reschedule self for x ms in future

    last_temperature = DS18B20_read_temperature();
}

float get_temp() {
  return last_temperature;
}
