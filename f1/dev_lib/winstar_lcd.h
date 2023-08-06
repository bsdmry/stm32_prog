#ifndef _H_WINSTAR_LCD
#define _H_WINSTAR_LCD

#include <stdint.h>
#include <stdlib.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <string.h>
#include <math.h>

#define WINSTAR_PCF8574_ADDR 0x20 //PCF8574
#define WINSTAR_PCF8574T_ADDR 0x27 //PCF8574T
#define WINSTAR_PCF8574A_ADDR 0x3F
#define WINSTAR_DUMMY_ADDR 0x00

#define WINSTAR_ENABLE 0x04
#define WINSTAR_WIDTH 20   // Maximum characters per line
#define WINSTAR_CHR  1 // Mode - Sending data
#define WINSTAR_CMD  0 // Mode - Sending command
#define WINSTAR_LINE_1  0x80 // LCD RAM address for the 1st line
#define WINSTAR_LINE_2  0xC0 // LCD RAM address for the 2nd lin

#define WINSTAR_INTERFACE_I2C 0x0
#define WINSTAR_INTERFACE_SPI 0x1

typedef struct {
	uint8_t interface_type;
	uint32_t device;
	uint8_t i2c_addr;
	uint8_t backlight;
} winstar_lcd;
winstar_lcd* winstar_init(uint8_t interface_type, uint32_t device, uint8_t i2c_addr);
void winstar_write_byte(winstar_lcd* disp, uint8_t byte, uint8_t flag);
void winstar_write(winstar_lcd* disp, uint8_t data);
void winstar_print (winstar_lcd* disp, char *msg,  uint8_t pos);
void winstar_display(winstar_lcd* disp, char *msg, uint8_t row,  uint8_t pos);
void winstar_display_numeric(winstar_lcd* disp, int32_t num, uint8_t area_size, uint8_t row,  uint8_t pos);

#endif
