#ifndef _H_CH455
#define _H_CH455
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <libopencm3/stm32/gpio.h>
#define CFG_REG 0x48
#define DIG0_REG 0x68
#define DIG1_REG 0x6A
#define DIG2_REG 0x6C
#define DIG3_REG 0x6E
#define READ_REG 0x4F

#define CHECK_FLASHING_BIT(x) (x & (1UL << 0) )
#define SET_FLASHING_BIT(x) (x |= (1UL << 0) )
#define CLEAR_FLASHING_BIT(x) (x &= ~(1UL << 0) )


typedef struct {
	uint8_t brightness;
	uint32_t port;
	uint32_t sda_pin;
	uint32_t scl_pin;
	uint8_t* ledData;
	uint8_t* ledOptions;
	uint8_t blinkSwitch;
} ch455;

void ch455_send_bit(ch455* led, uint8_t bit);
void ch455_write(ch455* led, uint8_t cmd, uint8_t data);
void ch455_write_cfg(ch455* led);
void ch455_set_brightness(ch455* led, uint8_t level);
void ch455_set_flashing_bitset(ch455* led, uint8_t bitset);
ch455* ch455_init(uint32_t port, uint32_t sda_pin, uint32_t scl_pin);
uint8_t ch455_char2bitmap(char c);
void ch455_display_data(ch455* led);
void ch455_send_text(ch455* led, char* text, uint8_t pos, uint8_t instantUpdate);
//void ch455_send_number(ch455* led, uint16_t num,  uint8_t instantUpdate);
void ch455_send_number(ch455* led, uint16_t num, uint8_t pos, uint8_t len,  uint8_t instantUpdate);
void ch455_send_fraction(ch455* led, uint16_t int_part, uint8_t int_pos, uint8_t int_len, uint16_t frac_part, uint8_t frac_pos, uint8_t frac_len, uint8_t instantUpdate);
#endif
