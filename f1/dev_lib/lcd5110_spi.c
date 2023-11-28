#include "lcd5110_spi.h"
#include "spi.h"
//6x14 chars

lcd5110* lcd5110_init(uint32_t device, uint32_t rst_port, uint32_t rst_pin, uint32_t dc_port, uint32_t dc_pin){
	lcd5110* disp = malloc(sizeof(lcd5110));
	disp->device = device;
	disp->rst_port = rst_port;
	disp->rst_pin = rst_pin;
	disp->dc_port = dc_port;
	disp->dc_pin = dc_pin;

	rcc_periph_clock_enable(rst_port | dc_port);
	gpio_set_mode(dc_port, GPIO_MODE_OUTPUT_50_MHZ,  GPIO_CNF_OUTPUT_PUSHPULL, dc_pin);
	gpio_set_mode(rst_port, GPIO_MODE_OUTPUT_50_MHZ,  GPIO_CNF_OUTPUT_PUSHPULL, rst_pin);
	gpio_set(rst_port, rst_pin);
	spi_set_cs(device, 1);

	return disp;
}


lcd5110_fifo_textbox* lcd5110_fifo_textbox_init(uint8_t char_width, uint8_t char_height, uint8_t x, uint8_t y){
	lcd5110_fifo_textbox* box = malloc(sizeof(lcd5110_fifo_textbox));
	if (char_width <= 14){
		box->char_width = char_width;
	} else {
		box->char_width = 14;
	}
	if (char_height <= 6){
		box->char_height = char_height;
	} else {
		box->char_height = 6;
	}
	box->x = x;
	box->y = y;
	box->text_size = char_width*char_height;
	box->text = malloc(sizeof(uint8_t)*char_width*char_height);
	memset(box->text, 0x20, (sizeof(uint8_t)*char_width*char_height));
	box->text_index = 0;
	box->text_head = char_width*char_height - 1;

	return box;
}

void lcd5110_fifo_textbox_add(lcd5110_fifo_textbox* box, char* str){
	uint16_t str_len = strlen(str);
	for (uint16_t i =0; i < str_len; i++){
		if (str[i] == '\n'){
			while( (box->text_index+1) % box->char_width ){
				box->text[box->text_index] = 0x20;
				box->text_index++;
				if (box->text_index >= box->text_size){
					box->text_index = 0;
				}
				if (box->text_index == box->text_head){
					box->text_head++;
					if (box->text_head >= box->text_size){
						box->text_head = 0;
					}
				}
				
			}
		} else if (str[i] == '\r'){
			__asm("nop");

		} else {
			box->text[box->text_index] = (uint8_t)str[i];
			box->text_index++;
			if (box->text_index >= box->text_size){
				box->text_index = 0;
			}
			if (box->text_index == box->text_head){
				box->text_head++;
				if (box->text_head >= box->text_size){
					box->text_head = 0;
				}
			}
		}
	}
}

void lcd5110_fifo_textbox_print(lcd5110* disp, lcd5110_fifo_textbox* box){
	uint8_t read_ptr = box->text_head;
	for (uint8_t h = 0; h < box->char_height; h++){
		for(uint8_t w = 0; w < box->char_width; w++){
			lcd5110_set_xy(disp, box->x + w*8, box->y + h);

			lcd5110_write_char(disp, box->text[read_ptr]);
			read_ptr++;
			if (read_ptr >= box->text_size ){
				read_ptr = 0;
			}
		}
	}
	
}

void lcd5110_write(lcd5110* disp, uint8_t data, uint8_t mode){
	if(mode == LCD5110_CMD){
		gpio_clear(disp->dc_port, disp->dc_pin);
	}else{
		gpio_set(disp->dc_port, disp->dc_pin);
	}
	spi_set_cs(disp->device, 0);
	spi_xfer(disp->device, (uint16_t)data);
	spi_set_cs(disp->device, 1);
}

void lcd5110_reset(lcd5110* disp){
	gpio_clear(disp->rst_port, disp->rst_pin);
	gpio_set(disp->rst_port, disp->rst_pin);
}

void lcd5110_clear(lcd5110* disp){
	  for(int i = 0; i < 504; i++){
		  lcd5110_write(disp, 0x00, LCD5110_DATA);
	}
}

void lcd5110_write_char(lcd5110* disp, char c){
  for(int i = 0; i < 6; i++){
	  lcd5110_write(disp, ASCII[c - 0x20][i], LCD5110_DATA);
  }
}

void lcd5110_set_xy(lcd5110* disp, uint8_t x, uint8_t y){
	lcd5110_write(disp, 0x80 | x, LCD5110_CMD); //Column.
	lcd5110_write(disp, 0x40 | y, LCD5110_CMD); //Row.
}

void lcd5110_print_normal(lcd5110* disp, char *str, uint8_t size, uint8_t x, uint8_t y){
	lcd5110_set_xy(disp, x, y);
	for(int i = 0; i < size; i++){
		lcd5110_write_char(disp, str[i]);
	}
}

void lcd5110_setup(lcd5110* disp){
	 lcd5110_reset(disp);
	 lcd5110_write(disp, 0x21, LCD5110_CMD); //LCD extended commands.
	 //lcd5110_write(disp, 0xB8, LCD5110_CMD); //set LCD Vop(Contrast).
	 lcd5110_write(disp, 0xba, LCD5110_CMD); //set LCD Vop(Contrast).
	 lcd5110_write(disp, 0x04, LCD5110_CMD); //set temp coefficent.
	 lcd5110_write(disp, 0x14, LCD5110_CMD); //LCD bias mode 1:40.
	 lcd5110_write(disp, 0x20, LCD5110_CMD); //LCD basic commands.
	 lcd5110_write(disp, LCD5110_DISPLAY_NORMAL, LCD5110_CMD); //LCD normal.
	 lcd5110_clear(disp);
}
