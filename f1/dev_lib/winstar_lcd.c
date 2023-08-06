#include "winstar_lcd.h"

winstar_lcd* winstar_init(uint8_t interface_type, uint32_t device, uint8_t i2c_addr){
	
	winstar_lcd* disp = malloc(sizeof(winstar_lcd));
	disp->interface_type = interface_type;
	disp->backlight = 0;
	disp->device = device;
	disp->i2c_addr = i2c_addr;
       	winstar_write_byte(disp, 0x33, WINSTAR_CMD); // 110011 Initialise
        winstar_write_byte(disp, 0x32, WINSTAR_CMD); // 110010 Initialise
        winstar_write_byte(disp, 0x06, WINSTAR_CMD); // 000110 Cursor move direction
        winstar_write_byte(disp, 0x0C, WINSTAR_CMD); // 001100 Display On,Cursor Off, Blink Off
        winstar_write_byte(disp, 0x28, WINSTAR_CMD); // 101000 Data length, number of lines, font size
        winstar_write_byte(disp, 0x01, WINSTAR_CMD); // 000001 Clear display	
	return disp;
}

void winstar_write_byte(winstar_lcd* disp, uint8_t byte, uint8_t flag){
        uint8_t bits_high = (flag | (byte & 0xF0) | (disp->backlight << 3) );
        uint8_t bits_low = (flag | ((byte<<4) & 0xF0) | (disp->backlight << 3) );
        winstar_write(disp, bits_high);
        winstar_write(disp, (bits_high | WINSTAR_ENABLE));
        winstar_write(disp, (bits_high & ~WINSTAR_ENABLE));
        // Low bits
        winstar_write(disp, bits_low);
        winstar_write(disp, (bits_low | WINSTAR_ENABLE));
        winstar_write(disp, (bits_low & ~WINSTAR_ENABLE));
}

void winstar_write(winstar_lcd* disp, uint8_t data){
	if (disp->interface_type == WINSTAR_INTERFACE_I2C){
    		uint8_t rd_buf;
    		i2c_transfer7(disp->device, disp->i2c_addr, &data, 1, &rd_buf, 0);
	} else if (disp->interface_type == WINSTAR_INTERFACE_SPI){
		spi_xfer(disp->device, (uint16_t)data);
	}
}

void winstar_print (winstar_lcd* disp, char *msg,  uint8_t pos){
	uint8_t strSize = strlen(msg);
        winstar_write_byte(disp, pos, WINSTAR_CMD);
        for (uint8_t p = 0; p < strSize; p++){
                winstar_write_byte(disp, (uint8_t)msg[p], WINSTAR_CHR);
        }
}

void winstar_display(winstar_lcd* disp, char *msg, uint8_t row,  uint8_t pos){
	uint8_t loc = 0;
	uint8_t strSize = strlen(msg);
	if (row == 1){
		loc =(0x80) | ((pos) & 0x0f);
	}
	if (row == 2){
		loc =(0xC0) | ((pos) & 0x0f);
	}
        winstar_write_byte(disp, loc, WINSTAR_CMD);
        for (uint8_t p = 0; p < strSize; p++){
                winstar_write_byte(disp, (uint8_t)msg[p], WINSTAR_CHR);
        }
}

void winstar_display_numeric(winstar_lcd* disp, int32_t num, uint8_t area_size, uint8_t row,  uint8_t pos){
	uint8_t loc = 0;
	if (row == 1){
		loc =(0x80) | ((pos) & 0x0f);
	}
	if (row == 2){
		loc =(0xC0) | ((pos) & 0x0f);
	}
        winstar_write_byte(disp, loc, WINSTAR_CMD);
	if (num < 0) {
                winstar_write_byte(disp, (uint8_t)'-', WINSTAR_CHR);	
		num *= -1;
	}

	uint32_t div = pow(10, area_size);
	 while (div > 0) {
                //dig = num % div;
                winstar_write_byte(disp, (uint8_t)((num / div) + 48), WINSTAR_CHR);
		num = num % div;
                div /= 10;
        }	
}
