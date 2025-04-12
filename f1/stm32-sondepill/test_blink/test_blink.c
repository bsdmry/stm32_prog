#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "test_blink.h"

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_24MHZ]);
//	rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_HSI_24MHZ]);
	rcc_periph_clock_enable(RCC_GPIOC);
}

static void setup (void) {
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
}

int main(void)
{
	clock_setup();
	setup();
	while(1){
		for (int i = 0; i < 6000000; ++i) {
   		}
    	gpio_toggle(GPIOC, GPIO13);
	}
}
