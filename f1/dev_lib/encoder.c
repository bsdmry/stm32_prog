#include "encoder.h"

void rotary_encoder_tim1_setup(uint16_t max_value){
    rcc_periph_clock_enable(RCC_TIM1);
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_AFIO);

    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO8 | GPIO9);

    timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, 	TIM_CR1_DIR_UP);
    timer_set_period(TIM1, max_value*4);
    //timer_set_prescaler(TIM1, 360);
    timer_disable_preload(TIM1);

    timer_slave_set_mode(TIM1, TIM_SMCR_SMS_EM3);
    //timer_ic_set_prescaler(TIM1, TIM_IC1, TIM_IC_PSC_8 );
    //timer_ic_set_prescaler(TIM1, TIM_IC2, TIM_IC_PSC_8 );

    timer_ic_set_filter(TIM1, TIM_IC1,  TIM_IC_DTF_DIV_32_N_8 );
    timer_ic_set_filter(TIM1, TIM_IC2,  TIM_IC_DTF_DIV_32_N_8 );

    timer_ic_set_input(TIM1, TIM_IC1, TIM_IC_IN_TI1);
    timer_ic_set_input(TIM1, TIM_IC2, TIM_IC_IN_TI2);

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
