#include <libopencm3/stm32/rcc.h>
#include "synth_si5351.h"

static void clock_setup(void)
{
rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);
rcc_periph_clock_enable(RCC_GPIOA);
}

int main(void)
{
clock_setup();
}
