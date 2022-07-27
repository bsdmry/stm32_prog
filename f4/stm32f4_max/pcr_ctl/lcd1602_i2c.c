#include "lcd1602_i2c.h"
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <string.h>
#define LCD1602_ADDR 0x3F
//#define LCD1602_ADDR 0x7E
#define LCD1602_BACKLIGHT 0x08
#define LCD1602_ENABLE 0x04
#define LCD1602_WIDTH 20   // Maximum characters per line
#define LCD1602_CHR  1 // Mode - Sending data
#define LCD1602_CMD  0 // Mode - Sending command
#define LCD1602_LINE_1  0x80 // LCD RAM address for the 1st line
#define LCD1602_LINE_2  0xC0 // LCD RAM address for the 2nd lin

void lcd1602_i2c_setup(void)
{
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_I2C1);
//    i2c_reset(I2C1);
    gpio_set_af(GPIOB, GPIO_AF4, GPIO6 | GPIO7);
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO7);
    i2c_peripheral_disable(I2C1);
    //configure ANFOFF DNF[3:0] in CR1
    /* HSI is at 25Mhz */
    //i2c_set_speed(I2C1, i2c_speed_sm_100k, 25);
    i2c_set_speed(I2C1, i2c_speed_sm_100k, rcc_apb1_frequency/1000000);
    i2c_peripheral_enable(I2C1);
}

void lcd1602_i2c_write(uint8_t addr, uint8_t data){
    uint8_t rd_buf;
    i2c_transfer7(I2C1, addr, &data, 1, &rd_buf, 0);
}

void lcd1602_write_byte(uint8_t byte, uint8_t flag){
        // Send byte to data pins
        // bits = the data
        // mode = 1 for character
        //        0 for command
        uint8_t bits_high = (flag | (byte & 0xF0) | LCD1602_BACKLIGHT);
        uint8_t bits_low = (flag | ((byte<<4) & 0xF0) | LCD1602_BACKLIGHT);
        lcd1602_i2c_write(LCD1602_ADDR, bits_high);
        lcd1602_i2c_write(LCD1602_ADDR, (bits_high | LCD1602_ENABLE));
        lcd1602_i2c_write(LCD1602_ADDR, (bits_high & ~LCD1602_ENABLE));
        // Low bits
        lcd1602_i2c_write(LCD1602_ADDR, bits_low);
        lcd1602_i2c_write(LCD1602_ADDR, (bits_low | LCD1602_ENABLE));
        lcd1602_i2c_write(LCD1602_ADDR, (bits_low & ~LCD1602_ENABLE));
}

void lcd1602_print (unsigned char *msg, uint8_t num,  uint8_t pos){
        // Send string to display
        unsigned char message[16];
        memset(message, 0x20, sizeof(message)); //clears buffer
        memcpy(message, msg, num);
        lcd1602_write_byte(pos, LCD1602_CMD);
        for (uint8_t p = 0; p <= 15  ; p++){
                lcd1602_write_byte((uint8_t)message[p], LCD1602_CHR);
        }
}

void lcd1602_init(void){
        lcd1602_i2c_setup();
        lcd1602_write_byte(0x33, LCD1602_CMD); // 110011 Initialise
        lcd1602_write_byte(0x32, LCD1602_CMD); // 110010 Initialise
        lcd1602_write_byte(0x06, LCD1602_CMD); // 000110 Cursor move direction
        lcd1602_write_byte(0x0C, LCD1602_CMD); // 001100 Display On,Cursor Off, Blink Off
        lcd1602_write_byte(0x28, LCD1602_CMD); // 101000 Data length, number of lines, font size
        lcd1602_write_byte(0x01, LCD1602_CMD); // 000001 Clear display
}
