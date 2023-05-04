#ifndef I2C_MASTER_H
#define I2C_MASTER_H
#include <avr/io.h>
void i2c_master_init(uint8_t pullups, uint8_t prescaler);
uint8_t i2c_master_open_write(uint8_t adr);
uint8_t i2c_master_open_read(uint8_t adr);
uint8_t i2c_master_write(uint8_t towrite);
uint8_t i2c_master_read_next(void);
uint8_t i2c_master_read_last(void);
void i2c_master_close();
#endif