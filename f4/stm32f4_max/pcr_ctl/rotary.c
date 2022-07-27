#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include "rotary.h"
uint8_t fsm_old_state = 0;
uint32_t rotary_encoder_value = 0;
uint8_t down_cnt = 0;
uint8_t up_cnt = 0;
uint8_t rotary_step_up = 0;
uint8_t rotary_step_down = 0;

void rotary_encoder_clock_setup(void){
    rcc_periph_clock_enable(RCC_TIM3);
    rcc_periph_clock_enable(RCC_GPIOC);
}

void rotary_encoder_setup(void){
    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO6 | GPIO7);
    gpio_set_af(GPIOC, GPIO_AF2, GPIO6 | GPIO7);
    timer_set_period(TIM3, 0xFFFF);
    timer_set_prescaler(TIM3, 10);
    timer_direction_up(TIM3);
    timer_set_clock_division(TIM3, TIM_CR1_CKD_CK_INT);
    timer_disable_preload(TIM3);

    timer_slave_set_mode(TIM3, TIM_SMCR_SMS_EM3);
    timer_ic_set_prescaler(TIM3, TIM_IC1, TIM_IC_PSC_4 );
    timer_ic_set_prescaler(TIM3, TIM_IC2, TIM_IC_PSC_4 );

    timer_ic_set_filter(TIM3, TIM_IC1,  TIM_IC_DTF_DIV_32_N_8 );
    timer_ic_set_filter(TIM3, TIM_IC2,  TIM_IC_DTF_DIV_32_N_8 );
    
    timer_ic_set_input(TIM3, TIM_IC1, TIM_IC_IN_TI1);
    timer_ic_set_input(TIM3, TIM_IC2, TIM_IC_IN_TI2);

    timer_enable_counter(TIM3);
}

void rotary_encoder_gpio_setup(void){
    gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO6 | GPIO7);
    rcc_periph_reset_pulse(RST_TIM3); 
    timer_set_clock_division(TIM3, TIM_CR1_CKD_CK_INT);
    timer_set_prescaler(TIM3, 8399);
    timer_set_period(TIM3, 100); //~10.1ms
    timer_direction_up(TIM3);
    /* Enable TIM2 interrupt. */
    nvic_set_priority(NVIC_TIM3_IRQ, 13);
    nvic_enable_irq(NVIC_TIM3_IRQ);
    timer_enable_update_event(TIM3);
    timer_enable_irq(TIM3, TIM_DIER_UIE);
    timer_enable_counter(TIM3);
}

void rotary_encoder_init(void){
    rotary_encoder_clock_setup();
    rotary_encoder_setup();
}

void rotary_encoder_gpio_init(void){
    rotary_encoder_clock_setup();
    rotary_encoder_gpio_setup();
}

uint32_t rotary_encoder_get_value(void){
    return timer_get_counter(TIM3);
}

uint32_t rotary_encoder_gpio_get_value(void){
    return rotary_encoder_value >> 1;
}

void tim3_isr(void)
{
        timer_clear_flag(TIM3, TIM_SR_UIF);
        gpio_toggle(GPIOE, GPIO8);
        uint8_t fsm_new_state = (uint8_t)((gpio_get(GPIOC, GPIO6 | GPIO7)) >> 6);
        //(fsm_new_state & 0x1) ? gpio_set(GPIOE, GPIO9) : gpio_clear(GPIOE, GPIO9);
        //(fsm_new_state >> 1) ? gpio_set(GPIOE, GPIO10) : gpio_clear(GPIOE, GPIO10);
        switch (fsm_old_state){
            case 2: {
                if (fsm_new_state == 3) {
                    rotary_encoder_value++;
                    up_cnt++;
                }
                if (fsm_new_state == 0) {
                    rotary_encoder_value--;
                    down_cnt++;
                }
                break;
            }
            case 0: {
                if (fsm_new_state == 2) {
                    rotary_encoder_value++;
                    up_cnt++;
                }
                if (fsm_new_state == 1) {
                    rotary_encoder_value--;
                    down_cnt++;
                }
                break;
            }
            case 1: {
                if (fsm_new_state == 0) {
                    rotary_encoder_value++;
                    up_cnt++;
                }
                if (fsm_new_state == 3) {
                    rotary_encoder_value--;
                    down_cnt++;
                }
                break;
            }
            case 3: {
                if (fsm_new_state == 1) {
                    rotary_encoder_value++;
                    up_cnt++;
                }
                if (fsm_new_state == 2) {
                    rotary_encoder_value--;
                    down_cnt++;
                }
                break;
            }
        }
        if (up_cnt == 2){
            up_cnt = 0;
            rotary_step_up = 1;
            rotary_step_down = 0;
        }
        if (down_cnt == 2){
            down_cnt = 0;
            rotary_step_up = 0;
            rotary_step_down = 1;
        }
        fsm_old_state = fsm_new_state;
}
