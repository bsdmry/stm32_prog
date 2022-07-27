#include <libopencm3/stm32/rcc.h>
#include "ad985x.h"
#include "dds_test.h"

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);
}
int main(void)
{
	clock_setup();
	ad985x_init(SPI1, GPIOA, GPIO3);
//	ad9850_set_freq(SPI1, 7000000.0);
	ad9851_set_freq(SPI1, 7000000.0);
	while(1);
}
