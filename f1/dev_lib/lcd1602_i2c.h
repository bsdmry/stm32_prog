#ifndef _LCD1602_I2C_H
#define _LCD1602_I2C_H

#include <stdint.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <string.h>

#define LCD1602_ADDR 0x20 //PCF8574
//#define LCD1602_ADDR 0x27 //PCF8574T
//#define LCD1602_ADDR 0x3F
#define LCD1602_BACKLIGHT 0x08
#define LCD1602_ENABLE 0x04
#define LCD1602_WIDTH 20   // Maximum characters per line
#define LCD1602_CHR  1 // Mode - Sending data
#define LCD1602_CMD  0 // Mode - Sending command
#define LCD1602_LINE_1  0x80 // LCD RAM address for the 1st line
#define LCD1602_LINE_2  0xC0 // LCD RAM address for the 2nd lin

void lcd1602_i2c_write(uint32_t i2c, uint8_t addr, uint8_t data);
void lcd1602_write_byte(uint32_t i2c, uint8_t byte, uint8_t flag);
void lcd1602_print (uint32_t i2c, char *msg, uint8_t pos);
void lcd1602_display(uint32_t i2c, char *msg, uint8_t row,  uint8_t pos);
void lcd1602_display_numeric(uint32_t i2c, int32_t num, uint8_t area_size, uint8_t row,  uint8_t pos);
void lcd1602_init(uint32_t i2c);

#endif
