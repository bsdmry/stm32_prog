#ifndef H_F1_I2C
#define H_F1_I2C
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>


void i2c_1_setup(void);
void i2c_1_1_setup(void);
void i2c_write_reg(uint32_t i2c, uint8_t dev_addr, uint8_t reg, uint8_t *data, uint8_t data_len);

void i2c_write_reg_single(uint32_t i2c, uint8_t dev_addr, uint8_t reg, uint8_t data);

#endif
