#include "encoder.h"

void rotary_encoder_tim1_setup(uint16_t max_value){
    rcc_periph_clock_enable(RCC_TIM1);
    rcc_periph_clock_enable(RCC_GPIOA);

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO8 | GPIO9);
    gpio_set_af(GPIOA, GPIO_AF1, GPIO8 | GPIO9);

    timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, 	TIM_CR1_DIR_UP);
    timer_set_period(TIM1, max_value*4);
    timer_disable_preload(TIM1);

    timer_slave_set_mode(TIM1, TIM_SMCR_SMS_EM3);

    timer_ic_set_filter(TIM1, TIM_IC1,  TIM_IC_DTF_DIV_32_N_8 );
    timer_ic_set_filter(TIM1, TIM_IC2,  TIM_IC_DTF_DIV_32_N_8 );

    timer_ic_set_input(TIM1, TIM_IC1, TIM_IC_IN_TI1);
    timer_ic_set_input(TIM1, TIM_IC2, TIM_IC_IN_TI2);

    timer_set_counter(TIM1, 0);

    timer_enable_counter(TIM1);
}

uint32_t rotary_encoder_tim1_get_value(void){
    return (timer_get_counter(TIM1) >> 2);
}
void rotary_encoder_tim1_set_limit(uint32_t value){
    timer_set_period(TIM1, value*4);
}
void rotary_encoder_tim1_set_value(uint32_t value){
    timer_set_counter(TIM1, value*4);
}

void rotary_encoder_tim3_setup(uint16_t max_value){
    rcc_periph_clock_enable(RCC_TIM3);
    rcc_periph_clock_enable(RCC_GPIOC);

    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO6 | GPIO7);
    gpio_set_af(GPIOC, GPIO_AF2, GPIO6 | GPIO7);

    timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, 	TIM_CR1_DIR_UP);
    timer_set_period(TIM3, max_value*4);
    timer_disable_preload(TIM3);

    timer_slave_set_mode(TIM3, TIM_SMCR_SMS_EM3);

    timer_ic_set_filter(TIM3, TIM_IC1,  TIM_IC_DTF_DIV_32_N_8 );
    timer_ic_set_filter(TIM3, TIM_IC2,  TIM_IC_DTF_DIV_32_N_8 );

    timer_ic_set_input(TIM3, TIM_IC1, TIM_IC_IN_TI1);
    timer_ic_set_input(TIM3, TIM_IC2, TIM_IC_IN_TI2);

    timer_set_counter(TIM3, 0);

    timer_enable_counter(TIM3);
}
uint32_t rotary_encoder_tim3_get_value(void){
    return (timer_get_counter(TIM3) >> 2);
}
void rotary_encoder_tim3_set_limit(uint32_t value){
    timer_set_period(TIM3, value*4);
}
void rotary_encoder_tim3_set_value(uint32_t value){
    timer_set_counter(TIM3, value*4);
}
