#include "lcd1602_gpio.h"
#include "delay.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <string.h>

void lcd1602_gpio_setup(void)
{
    rcc_periph_clock_enable(RCC_GPIOD);
    gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, RS_PIN | RW_PIN | E_PIN | DATA0 | DATA1 | DATA2 | DATA3);
    gpio_clear(GPIOD,  RS_PIN | RW_PIN | E_PIN | DATA0 | DATA1 | DATA2 | DATA3);
}

void set_nibble(uint8_t byte){
     (byte & (1 << 0)) ? gpio_set(GPIOD, DATA0) : gpio_clear(GPIOD, DATA0);
     (byte & (1 << 1)) ? gpio_set(GPIOD, DATA1) : gpio_clear(GPIOD, DATA1);
     (byte & (1 << 2)) ? gpio_set(GPIOD, DATA2) : gpio_clear(GPIOD, DATA2);
     (byte & (1 << 3)) ? gpio_set(GPIOD, DATA3) : gpio_clear(GPIOD, DATA3);
}
void lcd1602_gpio_write_byte(uint8_t byte, uint8_t flag){
        // Send byte to data pins
        // bits = the data
        // mode = 1 for character
        //        0 for command
        uint8_t bits_high = (byte >> 4);
        uint8_t bits_low = (byte & 0x0F);
        flag ? gpio_set(GPIOD, RS_PIN) : gpio_clear(GPIOD, RS_PIN);
        
        set_nibble(bits_high);
        gpio_set(GPIOD, E_PIN);
        delay_us(1);
        gpio_clear(GPIOD, E_PIN);
        delay_us(200);
        set_nibble(bits_low);
        gpio_set(GPIOD, E_PIN);
        delay_us(1);
        gpio_clear(GPIOD, E_PIN);
        delay_ms(2);
}

void lcd1602_gpio_print (unsigned char *msg, uint8_t num,  uint8_t line){
        // Send string to display
        unsigned char message[16];
        memset(message, 0x20, sizeof(message)); //clears buffer
        memcpy(message, msg, num);
        lcd1602_gpio_write_byte(line, LCD1602_CMD);
        for (uint8_t p = 0; p <= 15  ; p++){
                lcd1602_gpio_write_byte((uint8_t)message[p], LCD1602_CHR);
        }
}

void lcd1602_gpio_print_offset (unsigned char *msg, uint8_t num,  uint8_t line, uint8_t offset){
        // Send string to display
        lcd1602_gpio_write_byte(line + offset, LCD1602_CMD);
        for (uint8_t p = 0; p < num  ; p++){
                lcd1602_gpio_write_byte((uint8_t)msg[p], LCD1602_CHR);
        }
}

void lcd1602_gpio_init(void){
        lcd1602_gpio_setup();
        lcd1602_gpio_write_byte(0x33, LCD1602_CMD); // 110011 Initialise
        lcd1602_gpio_write_byte(0x32, LCD1602_CMD); // 110010 Initialise
        lcd1602_gpio_write_byte(0x06, LCD1602_CMD); // 000110 Cursor move direction
        lcd1602_gpio_write_byte(0x0C, LCD1602_CMD); // 001100 Display On,Cursor Off, Blink Off
        lcd1602_gpio_write_byte(0x28, LCD1602_CMD); // 101000 Data length, number of lines, font size
        lcd1602_gpio_write_byte(0x01, LCD1602_CMD); // 000001 Clear display
}
