#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "timer_test.h"

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_TIM3);
}
static void tim_setup(void)
{
	rcc_periph_reset_pulse(RST_TIM3);
	timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	/*
	 * Please take note that the clock source for STM32 timers
	 * might not be the raw APB1/APB2 clocks.  In various conditions they
	 * are doubled.  See the Reference Manual for full details!
	 * In our case, TIM2 on APB1 is running at double frequency, so this
	 * sets the prescaler to have the timer run at 5kHz
	 */
//	uint32_t apb1 = rcc_apb1_frequency;
//	uint32_t apb2 = rcc_apb2_frequency;
	uint32_t hz = 8000;
	uint32_t prescaler = 48000000 / hz;
    	uint32_t period = 2;
    	while(prescaler>4800){
        	prescaler=prescaler/10;
        	period=period*10;
    	}

	timer_continuous_mode(TIM3);
	//timer_set_prescaler(TIM2, ((rcc_apb1_frequency * 2) / (1000 - 1) ));
	timer_set_prescaler(TIM3, prescaler -1 );
	timer_set_period(TIM3, period -1 );
	timer_set_counter(TIM3,0);
	/* Counter enable. */
	timer_enable_counter(TIM3);
	timer_enable_irq(TIM3, TIM_DIER_UIE);
	nvic_enable_irq(NVIC_TIM3_IRQ);
	//nvic_set_priority(NVIC_TIM3_IRQ, 1);


}

int main(void)
{
	clock_setup();
	tim_setup();
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO3);
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
	gpio_set(GPIOA, GPIO3);
	gpio_set(GPIOC, GPIO13);
	
	while (1) {
		;
	}
}
void tim3_isr(void)
{
	if (timer_interrupt_source(TIM3, TIM_SR_UIF)) {
		gpio_toggle(GPIOA, GPIO3);
		gpio_toggle(GPIOC, GPIO13);
		/* Clear compare interrupt flag. */
		timer_clear_flag(TIM3, TIM_SR_UIF);
	}	
}
