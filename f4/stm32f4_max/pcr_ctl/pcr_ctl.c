//#include <libopencm3/stm32/rcc.h>

#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "delay.h"
#include "usart_cfg.h"
#include "lcd1602_gpio.h"
#include "timer_regular.h"
#include "rotary.h"
extern uint8_t rx_buf;
extern uint8_t rx_data_len;
extern uint8_t rx_buf_ready;
extern uint8_t timer_regular_irq;
extern uint8_t rotary_step_up;
extern uint8_t rotary_step_down;
uint32_t frequency = 0;
static void clock_setup(void)
{
        rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
        enable_dwt_counter();
}

void print_frequency(void){
    uint8_t sFreq[16];
    memset(sFreq, 0x20, (sizeof(int8_t)*16));
    memcpy(&sFreq[13], (uint8_t*)" Hz", 3); 
    sFreq[0] = (uint8_t)((frequency / 1000000000) + 0x30);
    sFreq[1] = 46;
    sFreq[2] = (uint8_t)(((frequency / 100000000)%10) + 0x30);
    sFreq[3] = (uint8_t)(((frequency / 10000000)%10) + 0x30);
    sFreq[4] = (uint8_t)(((frequency / 1000000)%10) + 0x30);
    sFreq[5] = 46;
    sFreq[6] = (uint8_t)(((frequency / 100000)%10) + 0x30);
    sFreq[7] = (uint8_t)(((frequency / 10000)%10) + 0x30);
    sFreq[8] = (uint8_t)(((frequency / 1000)%10) + 0x30);
    sFreq[9] = 46;
    sFreq[10] = (uint8_t)(((frequency / 100)%10) + 0x30);
    sFreq[11] = (uint8_t)(((frequency / 10)%10) + 0x30);
    sFreq[12] = (uint8_t)((frequency%10) + 0x30);
    lcd1602_gpio_print(sFreq, 16, LCD1602_LINE_1);
}

int main(void)
{
    frequency = 1100666321;
    clock_setup();
    usart_init();
    lcd1602_gpio_init(); 
    timer_regular_init();
    rotary_encoder_gpio_init();
    print_frequency();
    usartWriteString((uint8_t *)"Hz opencm2!", 11);
    uint8_t scr_div = 0;
    rcc_periph_clock_enable(RCC_GPIOE); //DBG PIN
    gpio_mode_setup(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8); //DBG PIN
    while (1) {
        if (timer_regular_irq){
            timer_regular_irq = 0;
            if (rotary_step_up){
                frequency++;
                rotary_step_up = 0;
            }
            if (rotary_step_down){
                frequency--;
                rotary_step_down = 0;
            }
            if (scr_div >= 10){
                print_frequency();
                scr_div = 0;
            } else {
                scr_div++;
            }
        }
        __asm__("NOP");
    }

    return 0;
}
