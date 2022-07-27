#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "blink.h"

static void clock_setup(void)
{
rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);
rcc_periph_clock_enable(RCC_GPIOC);
}

static void gpio_setup(void)
{
        gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT,
                        GPIO_PUPD_NONE, GPIO13);
       // gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO4);
}


int main(void)
{
        int i;

        clock_setup();
        gpio_setup();
        /* Set two LEDs for wigwag effect when toggling. */
        gpio_set(GPIOC, GPIO13);

        while (1) {
                /* Toggle LEDs. */
                gpio_toggle(GPIOC, GPIO13);
                for (i = 0; i < 6000000; i++) { /* Wait a bit. */
                        __asm__("nop");
                }
        }

        return 0;
}

