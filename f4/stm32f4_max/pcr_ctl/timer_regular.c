#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include "timer_regular.h"
uint8_t timer_regular_irq = 0;
uint8_t timer_regular_started = 0;

void timer_regular_clock_setup(void){
    /* Enable TIM7 clock. */
    rcc_periph_clock_enable(RCC_TIM7);
    //rcc_periph_clock_enable(RCC_GPIOE); //DBG PIN
}

void timer_regular_setup(void){
    //gpio_mode_setup(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8); //DBG PIN
    //gpio_set(GPIOE, GPIO8);
    /* Reset TIM2 peripheral to defaults. */
    rcc_periph_reset_pulse(RST_TIM7);
    /*
     * Please take note that the clock source for STM32 timers
     * might not be the raw APB1/APB2 clocks.  In various conditions they
     * are doubled.  See the Reference Manual for full details!
     * In our case, TIM2 on APB1 is running at double frequency, so this
     * sets the prescaler to have the timer run at 5kHz
     */
    timer_set_prescaler(TIM7, 8399);
    timer_set_period(TIM7, 100); //~10.1ms
    /* Enable TIM2 interrupt. */
    nvic_set_priority(NVIC_TIM7_IRQ, 14);
    nvic_enable_irq(NVIC_TIM7_IRQ);
    timer_enable_update_event(TIM7);
    timer_enable_irq(TIM7, TIM_DIER_UIE);
    timer_enable_counter(TIM7);
}

void timer_regular_init(void){
    timer_regular_clock_setup();
    timer_regular_setup();
    timer_regular_started = 1;
}

void tim7_isr(void)
{
        timer_clear_flag(TIM7, TIM_SR_UIF);
        /* Toggle LED to indicate compare event. */
        //gpio_toggle(GPIOE, GPIO8);
        timer_regular_irq = 1;
}
