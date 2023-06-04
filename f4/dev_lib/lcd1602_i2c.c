#include "lcd1602_i2c.h"


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

void lcd1602_set_custom_char(uint32_t i2c, uint8_t char_index, uint8_t *bitmap){
	if (char_index < 8){
		lcd1602_write_byte(i2c, 0x40 + char_index * 8, LCD1602_CMD);
		for (uint8_t i=0; i<8; i++) lcd1602_write_byte(i2c, (uint8_t)bitmap[i], LCD1602_CHR);
	}
}

Lcd1602_hbarline* lcd1602_init_horizontal_barline(uint8_t max_value, uint8_t bar_len, char empty_char, char *level_chars){
	Lcd1602_hbarline* bl = malloc(sizeof(Lcd1602_hbarline));	
	bl->bar_string = malloc(max_value * sizeof(char));
	bl->empty_value = empty_char;
	bl->max_value = max_value;
	bl->bar_length = bar_len;
	memcpy(bl->level_values, level_chars, LCD1602_CHAR_WIDTH);
	bl->values_per_bar_line = bl->max_value / (bl->bar_length * LCD1602_CHAR_WIDTH);
	bl->div_left = bl->max_value % (bl->bar_length * LCD1602_CHAR_WIDTH);
	
	bl->first_bar_value = (bl->div_left + bl->values_per_bar_line) + bl->values_per_bar_line * (LCD1602_CHAR_WIDTH - 1);

	return bl;
}

void lcd1602_set_horizontal_barline_value(Lcd1602_hbarline *bl, uint8_t value){
	memset(bl->bar_string, bl->empty_value, bl->bar_length);
	uint8_t end_bar_flag = 0;
	for (uint8_t i =0; i < bl->bar_length; i++){
		if (end_bar_flag){
			bl->bar_string[i] = bl->empty_value; 
		} else {
			if (i ==0){
				if (value >= bl->first_bar_value){
					 bl->bar_string[i] = bl->level_values[LCD1602_CHAR_WIDTH - 1];
				} else if (value == 0){
					bl->bar_string[i] = bl->empty_value;
					end_bar_flag = 1;
				} else {
					for (uint8_t b = 0; b < LCD1602_CHAR_WIDTH; b++){
						if (b ==0) {
							if (value < (bl->div_left + bl->values_per_bar_line)){
								bl->bar_string[i] = bl->level_values[b];
								end_bar_flag = 1;
								break;
							}
						} else {
							if (value < ((b+1) * bl->values_per_bar_line + bl->div_left)){
								bl->bar_string[i] = bl->level_values[b];
								end_bar_flag = 1;
								break;
							} 
						}
					}
				}
			} else {
				if (value >= (bl->first_bar_value + i*bl->values_per_bar_line*LCD1602_CHAR_WIDTH)){
					bl->bar_string[i] = bl->level_values[LCD1602_CHAR_WIDTH - 1];
				} else {
					for (uint8_t b = 0; b < LCD1602_CHAR_WIDTH; b++){
						if (value < bl->first_bar_value + (i-1)*LCD1602_CHAR_WIDTH*bl->values_per_bar_line + bl->values_per_bar_line*(b+1)){
							bl->bar_string[i] = bl->level_values[b];
							end_bar_flag = 1;
							break;
						}
					}
				}
			}
		}
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

