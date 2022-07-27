#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/timer.h>

#include "../../dev_lib/ssd1306.h"
#include "ssd1306_test.h"

volatile uint8_t active_flag = 0;
volatile uint32_t main_cnt = 0;

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_TIM3);
	rcc_periph_clock_enable(RCC_GPIOC);
}
static void tim3_setup(void){
        rcc_periph_reset_pulse(RST_TIM3);
        timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

        uint32_t hz = 2;
        uint32_t prescaler = rcc_apb1_frequency / hz;
        uint32_t period = 2;

        while(prescaler>4800){
                prescaler=prescaler/10;
                period=period*10;
        }

        timer_disable_preload(TIM3);
        timer_continuous_mode(TIM3);

        timer_set_prescaler(TIM3, prescaler -1 );
        timer_set_period(TIM3, period - 1);
        timer_set_counter(TIM3, 0); //Cleanup start value
        timer_enable_counter(TIM3);
        timer_enable_irq(TIM3, TIM_DIER_UIE);
        nvic_enable_irq(NVIC_TIM3_IRQ);
}

void tim3_isr(void)
{
        if (timer_get_flag(TIM3, TIM_SR_UIF)) {
                active_flag = 1;
		main_cnt++;
                timer_clear_flag(TIM3, TIM_SR_UIF);
        }
}

int main(void)
{
	clock_setup();
	i2c_1_setup();
	tim3_setup();
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
        gpio_clear(GPIOC, GPIO13);
	while(1){
		if ((active_flag) && (main_cnt == 8)){
			setup_ssd1306(I2C1, OLED_HMODE);
			fill_screen(I2C1, 0);
			gpio_toggle(GPIOC, GPIO13);
			active_flag = 0;
		}
		if (main_cnt > 8){
			print_number(I2C1, 0, 0, main_cnt);
		}
	}
}
