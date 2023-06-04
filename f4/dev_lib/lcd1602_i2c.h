#ifndef _LCD1602_I2C_H
#define _LCD1602_I2C_H

#include <stdint.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <stdlib.h>
#include <string.h>

//#define LCD1602_ADDR 0x27
#define LCD1602_ADDR 0x3F
#define LCD1602_BACKLIGHT 0x08
#define LCD1602_ENABLE 0x04
#define LCD1602_WIDTH 20   // Maximum characters per line
#define LCD1602_CHR  1 // Mode - Sending data
#define LCD1602_CMD  0 // Mode - Sending command
#define LCD1602_LINE_1  0x80 // LCD RAM address for the 1st line
#define LCD1602_LINE_2  0xC0 // LCD RAM address for the 2nd lin
#define LCD1602_CHAR_WIDTH 5
			
typedef struct {
	char *bar_string;
        char level_values[LCD1602_CHAR_WIDTH];
        char empty_value;
        uint8_t max_value;
        uint8_t bar_length;
	uint8_t values_per_bar_line;
	uint8_t div_left;
	uint8_t first_bar_value;
} Lcd1602_hbarline;


void lcd1602_i2c_write(uint32_t i2c, uint8_t addr, uint8_t data);
void lcd1602_write_byte(uint32_t i2c, uint8_t byte, uint8_t flag);
void lcd1602_print (uint32_t i2c, char *msg, uint8_t pos);
void lcd1602_display(uint32_t i2c, char *msg, uint8_t row,  uint8_t pos);
void lcd1602_set_custom_char(uint32_t i2c, uint8_t char_index, uint8_t *bitmap);
Lcd1602_hbarline* lcd1602_init_horizontal_barline(uint8_t max_value, uint8_t bar_len, char empty_char, char *level_chars);
void lcd1602_set_horizontal_barline_value(Lcd1602_hbarline *bl, uint8_t value);
void lcd1602_init(uint32_t i2c);

#endif
