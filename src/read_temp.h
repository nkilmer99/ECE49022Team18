#ifndef READ_TEMP_H
#define READ_TEMP_H

int DS18B20_reset();
void onewire_write_bit(bool bit);
int onewire_read_bit();
void onewire_write_byte(uint8_t byte);
uint8_t onewire_read_byte();
void DS18B20_init();
void temp_worker();
float get_temp();

#endif
