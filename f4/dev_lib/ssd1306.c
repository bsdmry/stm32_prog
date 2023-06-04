#include "ssd1306.h"

uint8_t screen_buf[OLED_BUF_SIZE];
uint8_t fill_value = 0x00;

void setup_ssd1306(uint32_t i2c, uint8_t amode){
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_DISPLAYOFF); //display off
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_MEMORYMODE); //Set Memory Addressing Mode
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, amode);

	if ((amode == OLED_HMODE) | (amode == OLED_VMODE)){
		i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, 0x21); //Set columns address for Horizontal and Vertical address mode
		i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, 0); //---set low column address
		i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, 127); //---set high column address

		i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, 0x22); //Set pages for Horizontal and Vertical address mode
		i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, 0); //---set low page address
		i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, OLED_PAGES); //---set high page address


	}
	if (amode == OLED_PMODE){
		uint8_t caddr = 0x00;
		i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_SETLOWCOLUMN | (caddr & 0x0F)); //set low nibble of column address for Page Addressing Mode
		i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_SETHIGHCOLUMN | (caddr >> 4)); //set high nibble of column address for Page Addressing Mode
		i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_SETSTARTPAGE + 0); //Set Page Start Address for Page Addressing Mode,0-7
	}

	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_COMSCANDEC); //Set COM Output Scan Direction
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_SETSTARTLINE); //--set start line address
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_SETCONTRAST); //--set contrast control register
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, 0x8F);
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_SEGREMAP | 0x01); //--set segment re-map 0 to 127 to NO
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_NORMALDISPLAY); //--set normal display
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_SETMULTIPLEX); //--set multiplex ratio(1 to 64)
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, 0x1F); //
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_DISPLAYALLON_RESUME); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_SETDISPLAYOFFSET); //-set display offset
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, 0x00); //-not offset
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_SETDISPLAYCLOCKDIV); //--set display clock divide ratio/oscillator frequency
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, 0x80); //--set divide ratio
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_SETPRECHARGE); //--set pre-charge period
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, 0x22); //
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_SETCOMPINS); //--set com pins hardware configuration
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, OLED_HW_PIN_CFG); //12 128x32 OLED
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_SETVCOMDETECT); //--set vcomh
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, 0x20); //0x20,0.77xVcc
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_CHARGEPUMP); //--set DC-DC enable
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, 0x14); //
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_DISPLAYON); //--turn on SSD1306 panel

	memset(screen_buf, 0x00, (sizeof(int8_t)*OLED_BUF_SIZE));
	fill_screen(i2c, amode);
}

void fill_screen(uint32_t i2c, uint8_t amode){

	if ((amode == OLED_HMODE) | (amode == OLED_VMODE)){
		memset(screen_buf, fill_value, (sizeof(int8_t)*OLED_BUF_SIZE));
		set_xy(i2c, 0, 0, amode);
		for (uint16_t i = 0; i < OLED_BUF_SIZE; i++){
			i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_DATA, screen_buf[i]);
		}
	}
	if (amode == OLED_PMODE){
		for (uint8_t m = 0; m < OLED_PAGES; m++) {
			set_xy(i2c, 0, m, amode);
			for (uint16_t i = 0; i < OLED_WIDTH; i++){
				i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_DATA, 0xa5);
			}
		}

	}

}

void set_area(uint32_t i2c, uint8_t x, uint8_t width, uint8_t y, uint8_t height){
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, 0x21); //Set columns address for Horizontal and Vertical address mode
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, x); //---set low column address
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, x + width - 1); //---set high column address

	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, 0x22); //Set pages for Horizontal and Vertical address mode
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, y); //---set low page address
	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, y + height -1); //---set high page address
}

void set_xy(uint32_t i2c, uint8_t x, uint8_t y, uint8_t amode){
	if ((amode == OLED_HMODE) | (amode == OLED_VMODE)){
		set_area(i2c, x, OLED_WIDTH, y, OLED_PAGES);
	}
	if (amode == OLED_PMODE){
		i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_SETLOWCOLUMN | (x & 0x0F)); //set low nibble of column address for Page Addressing Mode
		i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_SETHIGHCOLUMN | (x >> 4)); //set high nibble of column address for Page Addressing Mode
		i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_CMD, SSD1306_SETSTARTPAGE + y); //Set Page Start Address for Page Addressing Mode,0-7
	}
}

void ssd1306_display_char_8x16(uint32_t i2c, uint8_t x, uint8_t y,  uint8_t* bitmap){	
	set_area(i2c, x, 8, y, 2);
        for (uint8_t i = 0; i < 16; i++){
        	i2c_write_reg_single(i2c, SSD1306_I2C_ADDR, SSD1306_DATA, bitmap[i]);
        }
}


uint8_t ssd1306_fill_char_mapping_array(uint8_t* char_map_indexes , int32_t number, uint8_t area_size, uint8_t zerolead_flag){
        uint8_t negative = 0;
        uint8_t overflow = 0;
        uint8_t number_len = 0;
        //uint8_t char_map_indexes[11] = {0};
        if (area_size > 11) { area_size = 11; }
        if (number < 0) { negative = 1; number = -number; }

        //Fill output array by default values
        for (uint8_t i = 0; i< 11; i++){
                if (zerolead_flag){
                        char_map_indexes[i] = 0;
                } else {
                        char_map_indexes[i] = OLED1306_TABLE_DIG_BLANK_SIGN_INDEX;
                }
        }

        for(uint8_t p = 0; p < 11; p++){                                                                                        
                char_map_indexes[p] = (number % 10);                                                                                                                                                                                                                           
                number /= 10;                                                                                                         
                if (number == 0){                                                                                                     
                        number_len = p+1;                                                                                              
                        break;                                                                                                         
                }                                                                                                                      
        }
         //Is area_size enough for current number
        if (negative){
                if ((number_len + 1) > area_size){
                        overflow = 1;
                }
        } else {
                if (number_len > area_size){
                        overflow = 1;
                }
        }
        // Place overflow sign at the end
        if (overflow){
                char_map_indexes[area_size-1] = OLED1306_TABLE_DIG_OVERFLOW_SIGN_INDEX;
        } else {
                //Place sign if exist
                if (negative){
                        if (zerolead_flag){
                                char_map_indexes[area_size-1] = OLED1306_TABLE_DIG_NEGATIVE_SIGN_INDEX;
                        } else {
                                char_map_indexes[number_len] = OLED1306_TABLE_DIG_NEGATIVE_SIGN_INDEX;
                        }
                }
        }

       	return area_size-1;
}


void ssd1306_send_numeric_8x16(uint32_t i2c, int32_t number, uint8_t x, uint8_t y, uint8_t area_size, uint8_t zerolead_flag){
	uint8_t char_mapping_array[11] = {0};
	uint8_t startIndex = ssd1306_fill_char_mapping_array(char_mapping_array, number, area_size, zerolead_flag);
        for(int8_t p = startIndex; p >= 0; p--){
		uint8_t inv_index = (startIndex - p);
		uint8_t char_index = char_mapping_array[p];

		ssd1306_display_char_8x16(i2c,  x + inv_index*8, y, digits_8x16[char_index]);
        }
}

void ssd1306_send_string_8x16(uint32_t i2c, char* string, uint8_t x, uint8_t y){
	uint8_t strSize = strlen(string);
        for(int8_t p = 0; p < strSize; p++){	
		uint8_t char_index = 0;
		if ((uint8_t)(string[p]) >= 32){
			char_index = (uint8_t)(string[p]) - 32;
		}

		ssd1306_display_char_8x16(i2c,  x + p*8, y, ascii_8x16[char_index]);
        }

}


