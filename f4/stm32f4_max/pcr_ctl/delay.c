#include <libopencm3/cm3/dwt.h>
#include <libopencm3/stm32/rcc.h>
#include "delay.h"
void enable_dwt_counter(void){
    dwt_enable_cycle_counter();
}

uint32_t delta(uint32_t t0, uint32_t t1)
{
    return (t1 - t0);
}

void delay_us(uint32_t us)
{
        uint32_t t0 =  dwt_read_cycle_counter();
        uint32_t us_count_tic =  us * (rcc_ahb_frequency/1000000);
        while (delta(t0, dwt_read_cycle_counter()) < us_count_tic) ;
}

void delay_ms(uint32_t ms)
{
        uint32_t t0 =  dwt_read_cycle_counter();
        uint32_t ms_count_tic =  ms * (rcc_ahb_frequency/1000);
        while (delta(t0, dwt_read_cycle_counter()) < ms_count_tic) ;
}
