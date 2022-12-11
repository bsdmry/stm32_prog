#include <libopencm3/stm32/rcc.h>
#include <stdint.h>
#include <inttypes.h>
#include "i2c.h"
#include "si5351.h"
#include "synth_si5351.h"

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);
	rcc_periph_clock_enable(RCC_GPIOA);
}

int main(void)
{
	clock_setup();
	i2c_1_setup();
	si5351_init(I2C1, 0);
	si5351_setup_clk2(I2C1, 1000000, SI5351_DRIVE_STRENGTH_2MA);
	si5351_enable_outputs(I2C1, SI5351_OUT_CLK_2);
	while(1){
	
	}
}
