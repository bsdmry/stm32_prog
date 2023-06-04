#ifndef H_DWT_DELAY
#define H_DWT_DELAY

#include <libopencm3/cm3/dwt.h>
#include <libopencm3/stm32/rcc.h>
uint32_t dwt_setup(void);
void dwt_delay_us(volatile uint32_t au32_microseconds);
void dwt_delay_ms(volatile uint32_t au32_milliseconds);

#endif
