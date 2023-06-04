#ifndef _H_M62429
#define _H_M62429
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <libopencm3/stm32/gpio.h>

#define M62429_CH1 0
#define M62429_CH2 1

typedef struct {
	uint8_t ch1_volume;
	uint8_t ch2_volume;
	uint32_t port;
	uint32_t data_pin;
	uint32_t clock_pin;
} m62429;

m62429* m62429_init(uint32_t port, uint32_t sda_pin, uint32_t scl_pin);
void m62429_send_bit(m62429* sw, uint8_t bit);
void m62429_set_volume(m62429* sw, uint8_t channel, uint8_t volume);
#endif
