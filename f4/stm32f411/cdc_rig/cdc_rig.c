#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include "cdc_rig.h"
#include <inttypes.h>
#include <stdio.h>
#include "usart.h"

volatile uint8_t show_msg = 0;
volatile uint8_t msg_cnt = 0;

RigState rig = {.ptt = CAT_PTT_RX, .freqA = 1000000, .freqB = 1000000, .mode = CAT_MODE_LSB, .smeter = 75, .vfo = CAT_VFO_A };

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_TIM3);
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
		if (msg_cnt == 5){
                	show_msg = 1;
			msg_cnt = 0;
		} else {
			msg_cnt++;
		}
                timer_clear_flag(TIM3, TIM_SR_UIF);
        }
}


void set_ptt(uint8_t state){ rig.ptt = state; }
uint8_t get_ptt(void){ return rig.ptt; }
void set_freq(long f){ if (rig.vfo == CAT_VFO_A) {rig.freqA = f;} else {rig.freqB = f; } }
long get_freq(void){ return (rig.vfo == CAT_VFO_A) ? rig.freqA : rig.freqB; }
void set_mode(uint8_t state) { rig.mode = state; }
uint8_t get_mode(void){ return rig.mode; }
void set_split(uint8_t state) { __asm__("nop"); }
void AtoB(void) { rig.freqB = rig.freqA; }
void swap_vfo(void) {
	long f = rig.freqA;
	rig.freqA = rig.freqB;
	rig.freqB = f;
}
void set_vfo(uint8_t state) { rig.vfo = state; }
uint8_t get_smeter(void) { return rig.smeter; }

void show_rig(void){
	char blength[56];
	sprintf(blength, "PTT: %03"PRIu8" VFO: %03"PRIu8" VFO_A: %03"PRIi32" VFO_B: %03"PRIi32"\r\n", rig.ptt, rig.vfo, rig.freqA, rig.freqB);
	usart_send_string(USART2, blength, 56);
	show_msg = 0;	
}

void bind_handlers(void){
	ic746_add_handler_set_ptt(set_ptt);
	ic746_add_handler_get_ptt(get_ptt);
	ic746_add_handler_get_freq(get_freq);
	ic746_add_handler_set_freq(set_freq);
	ic746_add_handler_get_mode(get_mode);
	ic746_add_handler_set_mode(set_mode);
	ic746_add_handler_split(set_split);
	ic746_add_handler_AtoB(AtoB);
	ic746_add_handler_swap_vfo(swap_vfo);
	ic746_add_handler_set_vfo(set_vfo);
	ic746_add_handler_get_smeter(get_smeter);
}

int main(void)
{
	clock_setup();
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
        gpio_set(GPIOC, GPIO13);
	init_usb_cdc(0);
	usart_setup(USART2, 115200, 8, USART_STOPBITS_1, USART_MODE_TX, USART_PARITY_NONE, USART_FLOWCONTROL_NONE );
	bind_handlers();
	tim3_setup();
	while(1){
		if (show_msg) { show_rig(); }
		if (tx_ring_buffer.length != 0) {
		  	usart_send_string(USART2, "TX1\n",5);	
			cdcacm_data_tx(usbd_dev_g); 
		}
		if (rx_ring_buffer.length != 0) { 
		  	usart_send_string(USART2, "RX1\n",5);	
			uint8_t r = ic746_get_command();
			if (tx_ring_buffer.length != 0) { 
		  		usart_send_string(USART2, "TX2\n",5);	
				cdcacm_data_tx(usbd_dev_g); 
			}
			if (r) {
				usart_send_string(USART2, "PARSE!\n",8);
				ic746_parse_command(); 
			}
		}
	};
}
