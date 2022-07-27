#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include "cdc_command.h"
#include <inttypes.h>
#include <stdio.h>
#include "usb_cdc.h"
#include "usart.h"

volatile uint8_t show_msg = 0;
volatile uint8_t msg_cnt = 0;
uint8_t rigcmd[CAT_CMD_BUF_LENGTH] = {0};
Rig746 ic746_rig = {.cmdbuffer = rigcmd, .fsm_state = CAT_RCV_WAITING, .cmdlen = 0, .maxlen = CAT_CMD_BUF_LENGTH};

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

uint8_t check_command(void){
	uint8_t bt;
	uint8_t cmdRcvd = 0;
	uint8_t length = 0;
	while ((rx_ring_buffer.length != 0) && !cmdRcvd){
		buf_pop_u8(&rx_ring_buffer, &bt);
		//
		char s[12];
		sprintf(s, "BYTE 0x%02"PRIx8"\r\n", bt);
		usart_send_string(USART2, s, 12);
		//
		switch(ic746_rig.fsm_state){
			case CAT_RCV_WAITING:
				usart_send_string(USART2, "FSM_FIST_PRE\n", 14);
				if (bt == CAT_PREAMBLE) { ic746_rig.fsm_state = CAT_RCV_INIT; } break;
			case CAT_RCV_INIT:
				usart_send_string(USART2, "FSM_SECO_PRE\n", 14);
				if (bt == CAT_PREAMBLE) {
					ic746_rig.fsm_state = CAT_RCV_RECEIVING;
				} else {
					ic746_rig.fsm_state = CAT_RCV_WAITING;
					send_nack();
				}
				break;
			case CAT_RCV_RECEIVING:
				switch (bt){
					case CAT_EOM:
						usart_send_string(USART2, "FSM_RECV_END\n", 14);
						ic746_rig.fsm_state = CAT_RCV_WAITING;
						cmdRcvd = 1;
						ic746_rig.cmdlen = length;
						length = 0;
						send_echo();
						break;
					default:
						usart_send_string(USART2, "FSM_RCV_DATE\n", 14);
						if (length <= ic746_rig.maxlen){
							ic746_rig.cmdbuffer[length] = bt;
							length++;
						} else {
							ic746_rig.fsm_state = CAT_RCV_WAITING;
							length = 0;
							clean_command_buffer();
							send_nack();
						}
						break;
				}
				break;
		}
	//
	char sa[22];
	sprintf(sa, "RXB L: %02"PRIu8" cmdRcv: %02"PRIu8"\n", rx_ring_buffer.length, cmdRcvd);
	usart_send_string(USART2, sa, 22);
	//

	}
	return cmdRcvd;
}

void clean_command_buffer(void){
	for (uint8_t i = 0; i < ic746_rig.maxlen; i++){
		ic746_rig.cmdbuffer[i] = 0;
	}
	ic746_rig.cmdlen = 0;

}

void send_echo(void){
	uint8_t * resp = malloc(ic746_rig.cmdlen+3);
	//uint8_t resp[ic746_rig.cmdlen+3] = {0};
	resp[0] = CAT_PREAMBLE;
	resp[1] = CAT_PREAMBLE;
	for (uint8_t i = 0; i < ic746_rig.cmdlen; i++ ){
		resp[i+2] = ic746_rig.cmdbuffer[i];
	}
	resp[ic746_rig.cmdlen+2] = CAT_EOM;
	//
	usart_send_string(USART2, "SEND ECHO: ", 12);
	for (uint8_t i = 0; i < ic746_rig.cmdlen+3; i++){
		char s[7];
		sprintf(s, ">0x%02"PRIx8" ", resp[i]);
		usart_send_string(USART2, s, 7);
	}
	usart_send_string(USART2, "\r\n", 3);
	//
	cdc_send(resp, ic746_rig.cmdlen+3);
	free(resp);
}

void send_response(uint8_t cmdlen){
	uint8_t * resp = malloc(cmdlen+3);
	//uint8_t resp[cmdlen+3] = {0};
	resp[0] = CAT_PREAMBLE;
	resp[1] = CAT_PREAMBLE;
	resp[2] = ic746_rig.cmdbuffer[CAT_IX_FROM_ADDR];
	resp[3] = ic746_rig.cmdbuffer[CAT_IX_TO_ADDR];
	for (uint8_t i = CAT_IX_CMD; i < cmdlen; i++ ){
		resp[i+2] = ic746_rig.cmdbuffer[i];
	}
	resp[cmdlen+2] = CAT_EOM;
	//
	usart_send_string(USART2, "SEND RESP: ", 12);
	for (uint8_t i = 0; i < cmdlen+3; i++){
		char s[7];
		sprintf(s, ">0x%02"PRIx8" ", resp[i]);
		usart_send_string(USART2, s, 7);
	}
	usart_send_string(USART2, "\r\n", 3);
	//

	cdc_send(resp, cmdlen+3);
	free(resp);
}



void send_nack(void){
	usart_send_string(USART2, "SEND NACK\n", 11);
	uint8_t nack[6] = {CAT_PREAMBLE, CAT_PREAMBLE, CAT_CTRL_ADDR, CAT_RIG_ADDR, CAT_NACK, CAT_EOM};
	clean_command_buffer();
	cdc_send(nack, 6);
}

void send_ack(void){
	usart_send_string(USART2, "SEND ACK\n", 10);
	uint8_t ack[6] = {CAT_PREAMBLE, CAT_PREAMBLE, CAT_CTRL_ADDR, CAT_RIG_ADDR, CAT_ACK, CAT_EOM};
	clean_command_buffer();
	cdc_send(ack, 6);
}

int main(void)
{
	clock_setup();
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
        gpio_set(GPIOC, GPIO13);
	init_usb_cdc(0);
	usart_setup(USART2, 115200, 8, USART_STOPBITS_1, USART_MODE_TX, USART_PARITY_NONE, USART_FLOWCONTROL_NONE );
	tim3_setup();
	while(1){
		//if (show_msg) { show_rig(); }
		if (tx_ring_buffer.length != 0) {
		  	usart_send_string(USART2, "TX1\n",5);	
			cdcacm_data_tx(usbd_dev_g); 
		}
		if (rx_ring_buffer.length != 0) { 
		  	usart_send_string(USART2, "RX1\n",5);	
			uint8_t r = check_command();
			if (tx_ring_buffer.length != 0) { 
		  		usart_send_string(USART2, "TX2\n",5);	
				cdcacm_data_tx(usbd_dev_g); 
			}
			if (r) {
				usart_send_string(USART2, "PARSE!\n",8);
				parse_command(); 
			}
		}
	};
}
