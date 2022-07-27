#ifndef SSD1306_H_
#define SSD1306_H_
#include "i2c.h"
#include "/mnt/nfs/Work/stm32_include/oled1306_font.h"
#include <string.h>


#define SSD1306_SETLOWCOLUMN 0x00 /** Set Lower Column Start Address for Page Addressing Mode. */
#define SSD1306_SETHIGHCOLUMN 0x10 /** Set Higher Column Start Address for Page Addressing Mode. */
#define SSD1306_MEMORYMODE 0x20 /** Set Memory Addressing Mode. */
#define SSD1306_SETSTARTLINE 0x40 /** Set display RAM display start line register from 0 - 63. */
#define SSD1306_SETCONTRAST 0x81 /** Set Display Contrast to one of 256 steps. */
#define SSD1306_CHARGEPUMP 0x8D /** Enable or disable charge pump.  Follow with 0X14 enable, 0X10 disable. */
#define SSD1306_SEGREMAP 0xA0 /** Set Segment Re-map between data column and the segment driver. */
#define SSD1306_DISPLAYALLON_RESUME 0xA4 /** Resume display from GRAM content. */
#define SSD1306_DISPLAYALLON 0xA5 /** Force display on regardless of GRAM content. */
#define SSD1306_NORMALDISPLAY 0xA6 /** Set Normal Display. */
#define SSD1306_INVERTDISPLAY 0xA7 /** Set Inverse Display. */
#define SSD1306_SETMULTIPLEX 0xA8 /** Set Multiplex Ratio from 16 to 63. */
#define SSD1306_DISPLAYOFF 0xAE /** Set Display off. */
#define SSD1306_DISPLAYON 0xAF /** Set Display on. */
#define SSD1306_SETSTARTPAGE 0XB0 /**Set GDDRAM Page Start Address. */
#define SSD1306_COMSCANINC 0xC0 /** Set COM output scan direction normal. */
#define SSD1306_COMSCANDEC 0xC8 /** Set COM output scan direction reversed. */
#define SSD1306_SETDISPLAYOFFSET 0xD3 /** Set Display Offset. */
#define SSD1306_SETCOMPINS 0xDA /** Sets COM signals pin configuration to match the OLED panel layout. */
#define SSD1306_SETVCOMDETECT 0xDB /** This command adjusts the VCOMH regulator output. */
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5 /** Set Display Clock Divide Ratio/ Oscillator Frequency. */
#define SSD1306_SETPRECHARGE 0xD9 /** Set Pre-charge Period */
#define SSD1306_DEACTIVATE_SCROLL 0x2E /** Deactivate scroll */
#define SSD1306_NOP 0XE3 /** No Operation Command. */

#define SSD1306_I2C_ADDR	0x3C ///0x78
#define SSD1306_CMD	0x00
#define SSD1306_DATA	0x40
#define OLED_WIDTH 128

//#ifdef OLED128x32
#define OLED_HEIGHT 32
#define OLED_PAGES  4
#define OLED_HW_PIN_CFG 0x02
//#endif

//#ifdef OLED128x64
//#define OLED_HEIGHT 64
//#define OLED_PAGES  7
//#define OLED_HW_PIN_CFG 0x12
//#endif

#define OLED_COLUMNS OLED_WIDTH - 1
#define OLED_BUF_SIZE ((OLED_WIDTH * OLED_HEIGHT) / 8)
#define	OLED_PMODE  0x02 //10,Page Addressing Mode (RESET);
#define OLED_HMODE 0x00 //00,Horizontal Addressing Mode;
#define OLED_VMODE  0x01  //01,Vertical Addressing Mode;

typedef enum {
	NORMAL,
	INVERTED
}ChrMap;

void set_area(uint32_t i2c, uint8_t x, uint8_t width, uint8_t y, uint8_t height);
void set_xy(uint32_t i2c, uint8_t x, uint8_t y, uint8_t amode);
void setup_ssd1306(uint32_t i2c, uint8_t amode);
void fill_screen(uint32_t i2c, uint8_t amode);
void print_number(uint32_t i2c, uint8_t x, uint8_t y, uint32_t number);

#endif /* SSD1306_H_ */
