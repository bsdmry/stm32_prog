#ifndef H_F4_ENCODER
#define H_F4_ENCODER

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#define ROTATION_UP 1
#define ROTATION_DOWN 2

void rotary_encoder_tim1_setup(uint16_t max_value);
uint32_t rotary_encoder_tim1_get_value(void);
void rotary_encoder_tim1_set_limit(uint32_t value);
void rotary_encoder_tim1_set_value(uint32_t value);

void rotary_encoder_tim3_setup(uint16_t max_value);
uint32_t rotary_encoder_tim3_get_value(void);
void rotary_encoder_tim3_set_limit(uint32_t value);
void rotary_encoder_tim3_set_value(uint32_t value);
#endif
