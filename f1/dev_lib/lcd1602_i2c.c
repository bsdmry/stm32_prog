#include "lcd1602_i2c.h"
#include <math.h>


void lcd1602_i2c_write(uint32_t i2c, uint8_t addr, uint8_t data){
    uint8_t rd_buf;
    i2c_transfer7(i2c, addr, &data, 1, &rd_buf, 0);
}

void lcd1602_write_byte(uint32_t i2c, uint8_t byte, uint8_t flag){
        // Send byte to data pins
        // bits = the data
        // mode = 1 for character
        //        0 for command
        uint8_t bits_high = (flag | (byte & 0xF0) | LCD1602_BACKLIGHT);
        uint8_t bits_low = (flag | ((byte<<4) & 0xF0) | LCD1602_BACKLIGHT);
        lcd1602_i2c_write(i2c, LCD1602_ADDR, bits_high);
        lcd1602_i2c_write(i2c, LCD1602_ADDR, (bits_high | LCD1602_ENABLE));
        lcd1602_i2c_write(i2c, LCD1602_ADDR, (bits_high & ~LCD1602_ENABLE));
        // Low bits
        lcd1602_i2c_write(i2c, LCD1602_ADDR, bits_low);
        lcd1602_i2c_write(i2c, LCD1602_ADDR, (bits_low | LCD1602_ENABLE));
        lcd1602_i2c_write(i2c, LCD1602_ADDR, (bits_low & ~LCD1602_ENABLE));
}

void lcd1602_print (uint32_t i2c, char *msg,  uint8_t pos){
        // Send string to display
        //unsigned char message[16];
        //memset(message, 0x20, sizeof(message)); //clears buffer
        //memcpy(message, msg, num);
	uint8_t strSize = strlen(msg);
        lcd1602_write_byte(i2c, pos, LCD1602_CMD);
        for (uint8_t p = 0; p < strSize; p++){
                lcd1602_write_byte(i2c, (uint8_t)msg[p], LCD1602_CHR);
        }
}

void lcd1602_display(uint32_t i2c, char *msg, uint8_t row,  uint8_t pos){
	uint8_t loc = 0;
	uint8_t strSize = strlen(msg);
	if (row == 1){
		loc =(0x80) | ((pos) & 0x0f);
	}
	if (row == 2){
		loc =(0xC0) | ((pos) & 0x0f);
	}
        lcd1602_write_byte(i2c, loc, LCD1602_CMD);
        for (uint8_t p = 0; p < strSize; p++){
                lcd1602_write_byte(i2c, (uint8_t)msg[p], LCD1602_CHR);
        }
	
}

void lcd1602_display_numeric(uint32_t i2c, int32_t num, uint8_t area_size, uint8_t row,  uint8_t pos){
	uint8_t loc = 0;
	if (row == 1){
		loc =(0x80) | ((pos) & 0x0f);
	}
	if (row == 2){
		loc =(0xC0) | ((pos) & 0x0f);
	}
        lcd1602_write_byte(i2c, loc, LCD1602_CMD);
	if (num < 0) {
                lcd1602_write_byte(i2c, (uint8_t)'-', LCD1602_CHR);	
		num *= -1;
	}

	uint32_t div = pow(10, area_size);
	 while (div > 0) {
                //dig = num % div;
                lcd1602_write_byte(i2c, (uint8_t)((num / div) + 48), LCD1602_CHR);
		num = num % div;
                div /= 10;
        }	
}

void lcd1602_init(uint32_t i2c){
        lcd1602_write_byte(i2c, 0x33, LCD1602_CMD); // 110011 Initialise
        lcd1602_write_byte(i2c, 0x32, LCD1602_CMD); // 110010 Initialise
        lcd1602_write_byte(i2c, 0x06, LCD1602_CMD); // 000110 Cursor move direction
        lcd1602_write_byte(i2c, 0x0C, LCD1602_CMD); // 001100 Display On,Cursor Off, Blink Off
        lcd1602_write_byte(i2c, 0x28, LCD1602_CMD); // 101000 Data length, number of lines, font size
        lcd1602_write_byte(i2c, 0x01, LCD1602_CMD); // 000001 Clear display
}

