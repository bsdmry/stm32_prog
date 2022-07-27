#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "simple-blink.h"

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
	rcc_periph_clock_enable(RCC_GPIOC);
}

void setup () {
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
                    GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
}

void loop () {
    for (int i = 0; i < 1650000; ++i) {
            gpio_toggle(GPIOC, GPIO13);
    }
}

int main(void)
{
	clock_setup();
	setup();
	loop();
}
