#include "dwt.h"

uint32_t dwt_setup(void)
{
	dwt_enable_cycle_counter();
     	/* 3 NO OPERATION instructions */
     	__asm volatile ("nop");
     	__asm volatile ("nop");
  	__asm volatile ("nop");

  /* Check if clock cycle counter has started */
     if(dwt_read_cycle_counter())
     {
       return 0; /*clock cycle counter started*/
     }
     else
  {
    return 1; /*clock cycle counter not started*/
  }
}

inline void dwt_delay_us(volatile uint32_t au32_microseconds)
{
	uint32_t au32_initial_ticks = dwt_read_cycle_counter();
	uint32_t au32_ticks = (rcc_ahb_frequency / 1000000);
	au32_microseconds *= au32_ticks;
	while ((dwt_read_cycle_counter() - au32_initial_ticks) < au32_microseconds-au32_ticks);
}

void dwt_delay_ms(volatile uint32_t au32_milliseconds)
{
	uint32_t au32_initial_ticks = dwt_read_cycle_counter();
	uint32_t au32_ticks = (rcc_ahb_frequency / 1000);
	au32_milliseconds *= au32_ticks;
	while ((dwt_read_cycle_counter() - au32_initial_ticks) < au32_milliseconds);
}
