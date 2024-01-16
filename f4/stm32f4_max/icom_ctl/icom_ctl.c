#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
//include "/storage/Develop/gcc-arm-none-eabi-10.3-2021.10/arm-none-eabi/include/stdlib.h"
//#include <newlib.h>
#include <stdlib.h>
#include "dwt.h"
#include "icom_ctl.h"
#include "usart.h"
#include "usb_cdc.h"
#include "i2c.h"
#include "winstar_lcd.h"
#include "encoder.h"
#include "cmd.h"

#define BTN_PORT GPIOE
#define BTN_MOD GPIO12
#define BTN_A GPIO13
#define BTN_B GPIO14
#define BTN_C GPIO15

#define ROTBTN_PORT GPIOD
#define ROTBTN GPIO15

#define BLINKER_DIV 4

volatile uint8_t to_cdc_flag = 0;
volatile uint8_t screen_update_flag = 0;
volatile uint8_t screen_blinker_flag = 0;
volatile uint8_t screen_blinker_div = BLINKER_DIV;
uint8_t state_fsm = STATE_FSM_MAIN;
uint8_t encoder_fsm = ENCODER_FSM_FREQ;
volatile uint32_t last_encoder = 0; //encoder position for frequency

winstar_lcd* lcd;

char lcd_line1[17];
char lcd_line2[17];
uint16_t lcd_line1_blink_bitmap = 0x0000;
uint16_t lcd_line2_blink_bitmap = 0x0000;

winstar_hbarline* volume_lvl;
winstar_hbarline* signal_lvl;

uint8_t btn_mod_press = 0;
uint8_t btn_a_press = 0;
uint8_t btn_b_press = 0;
uint8_t btn_c_press = 0;
uint8_t btn_rot_press = 0;

uint8_t btn_mod_trg = 0;
uint8_t btn_a_trg = 0;
uint8_t btn_b_trg = 0;
uint8_t btn_c_trg = 0;
uint8_t btn_rot_trg = 0;

uint8_t v1_chr[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F};
uint8_t v2_chr[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F};
uint8_t v3_chr[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F};
uint8_t v4_chr[8] = {0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F};
uint8_t v5_chr[8] = {0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
uint8_t v6_chr[8] = {0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
uint8_t v7_chr[8] = {0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};

uint8_t ant_chr[8] = {0x04, 0x15, 0x0A, 0x04, 0x04, 0x04, 0x04, 0x04};
uint8_t snd_chr[8] = {0x01, 0x03, 0x0F, 0x0F, 0x0F, 0x03, 0x01, 0x00};

uint8_t b1_chr[8] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 };
uint8_t b2_chr[8] = {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18 };
uint8_t b3_chr[8] = {0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C };
uint8_t b4_chr[8] = {0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E };
uint8_t b5_chr[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOE);
	rcc_periph_clock_enable(RCC_GPIOD);
	rcc_periph_clock_enable(RCC_TIM4);
}

static void tim4_setup(void)
{
        rcc_periph_reset_pulse(RST_TIM4);
        timer_set_mode(TIM4, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
        uint32_t prescaler = 4200;
        uint32_t period = 2000;
        timer_continuous_mode(TIM4);
        timer_set_prescaler(TIM4, prescaler -1 );
        timer_set_period(TIM4, period -1 );
        timer_set_counter(TIM4,0);
        /* Counter enable. */
        timer_enable_counter(TIM4);
        timer_enable_irq(TIM4, TIM_DIER_UIE);
        nvic_enable_irq(NVIC_TIM4_IRQ);
	nvic_set_priority(NVIC_TIM4_IRQ, 5);
}



int main(void)
{
	clock_setup();
	dwt_setup();
	//gpio_mode_setup(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8);
        //gpio_set(GPIOE, GPIO8);
	init_usb_cdc(1);
	tim4_setup();

	setup_controls();
        usart_setup(USART1, 9600, 8, USART_STOPBITS_1, USART_MODE_TX_RX, USART_PARITY_NONE, USART_FLOWCONTROL_NONE );
        usart_setup(USART2, 115200, 8, USART_STOPBITS_1, USART_MODE_TX, USART_PARITY_NONE, USART_FLOWCONTROL_NONE );
	usart_send_blocking(USART2, '+'); //DBG
	nvic_enable_irq(NVIC_USART1_IRQ);
	usart_enable_rx_interrupt(USART1);
	nvic_set_priority(NVIC_USART1_IRQ, 2);

	rotary_encoder_tim3_setup(9);
	rotary_encoder_tim3_set_value(0);	

	i2c_1_1_setup();
	lcd = winstar_init(WINSTAR_INTERFACE_I2C, I2C1, 0x3F );
	lcd->backlight = 1;
	winstar_set_custom_char(lcd, 0, b1_chr);
	winstar_set_custom_char(lcd, 1, b2_chr);
	winstar_set_custom_char(lcd, 2, b3_chr);
	winstar_set_custom_char(lcd, 3, b4_chr);
	winstar_set_custom_char(lcd, 4, b5_chr);
	winstar_set_custom_char(lcd, 5, ant_chr);
	winstar_set_custom_char(lcd, 6, snd_chr);
	
	volume_lvl = winstar_init_horizontal_barline(255, 3, ' ', "\x00\x01\x02\x03\x04");
	signal_lvl = winstar_init_horizontal_barline(255, 7, '-', "\x00\x01\x02\x03\x04");

	memset(lcd_line1, '-', 16);
	memset(lcd_line2, '-', 16);
	winstar_display(lcd, lcd_line1, 1, 0);
	winstar_display(lcd, lcd_line2, 2, 0);



	while(1){
		if (to_cdc_flag){
			to_cdc_flag = 0;
			cdcacm_data_tx(usbd_dev_g);
		}
		if (rx_ring_buffer.length > 0){
			 usart_enable_tx_interrupt(USART1);
		}
		if (screen_update_flag){
			switch(state_fsm){
				case STATE_FSM_MAIN: show_main_screen(); break;
				case STATE_FSM_RECIVER_OPTIONS: show_reciever_options_screen(); break;
				case STATE_FSM_CONTROL_MODE: show_connection_screen(); break;
				default: show_main_screen(); break;
			}
			if (screen_blinker_flag){
				for (uint8_t p = 0; p < 16; p++){
					if ((lcd_line1_blink_bitmap) & (uint16_t)(1 << p)){
						lcd_line1[p] = ' ';
					}
					if ((lcd_line2_blink_bitmap) & (uint16_t)(1 << p)){
						lcd_line2[p] = ' ';
					}
				}
			}
			winstar_display(lcd, lcd_line1, 1, 0);
			winstar_display(lcd, lcd_line2, 2, 0);
			screen_update_flag = 0;

			
		}
		if (btn_mod_trg || btn_a_trg || btn_b_trg || btn_c_trg  || btn_rot_trg){
			handle_buttons();
		}
			handle_encoder();
	}
}

void setup_controls(void){
	gpio_mode_setup(BTN_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP , BTN_MOD | BTN_A | BTN_B | BTN_C);
	gpio_mode_setup(ROTBTN_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP , ROTBTN);
}

void show_main_screen(void){
	winstar_set_horizontal_barline_value(volume_lvl, int_rcvr_params.volume);
	winstar_set_horizontal_barline_value(signal_lvl, int_rcvr_params.signalLevel);

	memset(lcd_line1, '-', 16);
	memset(lcd_line2, '-', 16);

	memcpy(&lcd_line1, "\x05", 1);
	if (encoder_fsm == ENCODER_FSM_SQL){
        	int2str((uint32_t)int_rcvr_params.signalLevel, 3, &lcd_line1[1]);
		lcd_line1[4] = '|';
        	int2str((uint32_t)int_rcvr_params.squelch_level, 3, &lcd_line1[5]);
	} else {
		memcpy(&lcd_line1[1], signal_lvl->bar_string, 7);
	}
	memcpy(&lcd_line1[8], "\x06", 1);
	memcpy(&lcd_line1[9], volume_lvl->bar_string, 3);
	memcpy(&lcd_line1[12], char_freq_params.filter, 4);


	memcpy(lcd_line2, char_freq_params.freq, 4);
	lcd_line2[4] = '.';
	memcpy(&lcd_line2[5], &char_freq_params.freq[4], 3);
	lcd_line2[8] = '.';
	memcpy(&lcd_line2[9], &char_freq_params.freq[7], 3);

	memcpy(&lcd_line2[13], char_freq_params.modulation, 3);

}

void show_reciever_options_screen(void){
	if (int_rcvr_params.agc_state != 0){
		memcpy(lcd_line1, "AGC on   ", 9);
	} else {
		memcpy(lcd_line1, "AGC off  ", 9);
	}
	if (int_rcvr_params.attenuator_state != 0){
		memcpy(lcd_line1+9, "ATT on ", 7);
	} else {
		memcpy(lcd_line1+9, "ATT off", 7);
	}
	if (int_rcvr_params.noise_blanker_state != 0){
		memcpy(lcd_line2, "NOISE BLANK on  ", 16);
	} else {
		memcpy(lcd_line2, "NOISE BLANK off ", 16);
	}
}

void show_connection_screen(void){
	memcpy(lcd_line1, "  Control mode  ", 16);
	switch(int_rcvr_params.controlMode){
		case CONTROL_MODE_NONE: memcpy(lcd_line2, "      NONE      " , 16); break;
		case CONTROL_MODE_STANDALONE: memcpy(lcd_line2, "   STANDALONE   ", 16); break;
		case CONTROL_MODE_BRIDGE: memcpy(lcd_line2, " SERIAL BRIDGE  ", 16); break;
		default: memcpy(lcd_line2, "      NONE      ", 16); break;
	}
}

void handle_buttons(void){
	if (btn_mod_trg){
		switch(state_fsm){
			case STATE_FSM_MAIN: state_fsm = STATE_FSM_RECIVER_OPTIONS; break;
			case STATE_FSM_RECIVER_OPTIONS: state_fsm = STATE_FSM_CONTROL_MODE; break;
			case STATE_FSM_CONTROL_MODE: state_fsm = STATE_FSM_MAIN; break;
			default: state_fsm = STATE_FSM_MAIN; break;
		}
		btn_mod_trg = 0;
	}
	if (btn_a_trg){
		switch(state_fsm){
			case STATE_FSM_MAIN: change_mod(); break;
			case STATE_FSM_RECIVER_OPTIONS: switch_agc(); break;
			case STATE_FSM_CONTROL_MODE: set_control_mode_none(); break;
			default: break;
		}
		btn_a_trg = 0;
	}
	if (btn_b_trg){
		switch(state_fsm){
			case STATE_FSM_MAIN: change_filter(); break;
			case STATE_FSM_RECIVER_OPTIONS: switch_att(); break;
			case STATE_FSM_CONTROL_MODE: set_control_mode_standalone(); break;
			default: break;
		}
		btn_b_trg = 0;
	}
	if (btn_c_trg){
		switch(state_fsm){
			case STATE_FSM_MAIN: change_vol_sql_freq(); break;
			case STATE_FSM_RECIVER_OPTIONS: switch_nb(); break;
			case STATE_FSM_CONTROL_MODE: set_control_mode_bridge(); break;
			default: break;
		}
		btn_c_trg = 0;
	}
	if (btn_rot_trg){
		if (state_fsm == STATE_FSM_MAIN){
			switch(encoder_fsm){
				case ENCODER_FSM_FREQ: encoder_fsm = ENCODER_FSM_FREQ_HZ; 
						       lcd_line2_blink_bitmap = 0x0E00;
						       break;
				case ENCODER_FSM_FREQ_HZ: encoder_fsm = ENCODER_FSM_FREQ_KHZ; 
						       lcd_line2_blink_bitmap = 0x00E0;
						       break;
				case ENCODER_FSM_FREQ_KHZ: encoder_fsm = ENCODER_FSM_FREQ_MHZ; 
						       lcd_line2_blink_bitmap = 0x000E;
						       break;
				case ENCODER_FSM_FREQ_MHZ: encoder_fsm = ENCODER_FSM_FREQ_GHZ; 
						       lcd_line2_blink_bitmap = 0x0001;
						       break;
				case ENCODER_FSM_FREQ_GHZ: encoder_fsm = ENCODER_FSM_FREQ; 
						       lcd_line2_blink_bitmap = 0x0000;
						       break;
				default: break;
			}
		}
		btn_rot_trg = 0;
	}
}

static void dbg_transmit(uint32_t number){
        uint32_t div = 1000000000;
        usart_send_blocking(USART2, 'D');
        usart_send_blocking(USART2, '>');
        while (div > 0) {
                usart_send_blocking(USART2, ((uint8_t)(number / div) + 0x30));
                number = number % div;
                div /= 10;
        }

        usart_send_blocking(USART2, 0x0D);
        usart_send_blocking(USART2, 0x0A);
}

void step_change_freq(uint32_t current, uint32_t previous, uint32_t step_size){
	int32_t delta = current - previous;
	if (delta != 0){
		if ((previous == 9) && (current == 0)){
			delta = 1;
                } else if ((previous == 0) && (current == 9)){
                        delta = -1;
                }
		//dbg_transmit(previous);
		//dbg_transmit(current);
		int_rcvr_params.frequency = int_rcvr_params.frequency + (delta * step_size);
		if (int_rcvr_params.frequency < 10000){
			int_rcvr_params.frequency = 10000;
		}else if (int_rcvr_params.frequency > 1300000000){
			int_rcvr_params.frequency = 1300000000;
		}
		//dbg_transmit(int_rcvr_params.frequency);
		set_reciever_params(); 
	}
}

void handle_encoder(void){
	if (int_rcvr_params.controlMode == CONTROL_MODE_STANDALONE){
		uint32_t cv = 0;
		switch(encoder_fsm){
			case ENCODER_FSM_FREQ:
				cv = rotary_encoder_tim3_get_value();
				step_change_freq(cv, last_encoder, int_rcvr_params.step);
				last_encoder = cv;
				break;
			case ENCODER_FSM_FREQ_HZ:
				cv = rotary_encoder_tim3_get_value();
				step_change_freq(cv, last_encoder, 1);
				last_encoder = cv;
				break;
			case ENCODER_FSM_FREQ_KHZ:
				cv = rotary_encoder_tim3_get_value();
				step_change_freq(cv, last_encoder, 1000);
				last_encoder = cv;
				break;
			case ENCODER_FSM_FREQ_MHZ:
				cv = rotary_encoder_tim3_get_value();
				step_change_freq(cv, last_encoder, 1000000);
				last_encoder = cv;
				break;
			case ENCODER_FSM_FREQ_GHZ:
				cv = rotary_encoder_tim3_get_value();
				step_change_freq(cv, last_encoder, 1000000000);
				last_encoder = cv;
				break;

			case ENCODER_FSM_VOL:
				cv = rotary_encoder_tim3_get_value();
				if (int_rcvr_params.volume != (uint8_t)cv){
					if ((int_rcvr_params.volume == 255) && (cv == 0)){
						rotary_encoder_tim3_set_value(255);	
					} else if ((int_rcvr_params.volume == 0) && (cv == 255)){
						rotary_encoder_tim3_set_value(0);	
					} else {
						int_rcvr_params.volume = (uint8_t)cv;
						set_volume();
					}
				}
				break;
			case ENCODER_FSM_SQL:
				cv = rotary_encoder_tim3_get_value();
				if (int_rcvr_params.squelch_level != (uint8_t)cv){
					if ((int_rcvr_params.squelch_level == 255) && (cv == 0)){
						rotary_encoder_tim3_set_value(255);	
					} else if ((int_rcvr_params.squelch_level == 0) && (cv == 255)){
						rotary_encoder_tim3_set_value(0);	
					} else {
						int_rcvr_params.squelch_level = (uint8_t)cv;
						set_squelch();
					}
				}
				break;
			default: break;
		}
	}
}


void set_control_mode_standalone(void){
	int_rcvr_params.controlMode = CONTROL_MODE_STANDALONE;
	if (int_rcvr_params.wasInit == ICOM_HASNT_CFG){
		send_command("H101\r\n", 6);
		dwt_delay_ms(500);
		send_command("H101\r\n", 6);
		dwt_delay_ms(500);
		send_command("H101\r\n", 6);
		dwt_delay_ms(500);
		send_command("G105\r\n", 6);
		dwt_delay_ms(100);
		usart_disable(USART1);
		usart_setup(USART1, 38400, 8, USART_STOPBITS_1, USART_MODE_TX_RX, USART_PARITY_NONE, USART_FLOWCONTROL_NONE );
		nvic_enable_irq(NVIC_USART1_IRQ);
		usart_enable_rx_interrupt(USART1);
	//	dwt_delay_ms(250);
		send_command("G301\r\n", 6);	
	}
	dwt_delay_ms(50);
	set_reciever_params(); 
	dwt_delay_ms(50);
	set_squelch();
	dwt_delay_ms(50);
	set_volume();
	dwt_delay_ms(50);
}

void set_control_mode_bridge(void){
	if ((int_rcvr_params.wasInit == ICOM_HAS_CFG) && 
	(int_rcvr_params.controlMode == CONTROL_MODE_STANDALONE)){
		send_command("G103\r\n", 6);
		usart_disable(USART1);
		usart_setup(USART1, 9600, 8, USART_STOPBITS_1, USART_MODE_TX_RX, USART_PARITY_NONE, USART_FLOWCONTROL_NONE );
	}
	int_rcvr_params.controlMode = CONTROL_MODE_BRIDGE;
}

void set_control_mode_none(void){
	if ((int_rcvr_params.wasInit == ICOM_HAS_CFG) && 
	(int_rcvr_params.controlMode == CONTROL_MODE_STANDALONE)){
		send_command("G103\r\n", 6);
		usart_disable(USART1);
		usart_setup(USART1, 9600, 8, USART_STOPBITS_1, USART_MODE_TX_RX, USART_PARITY_NONE, USART_FLOWCONTROL_NONE );
		nvic_enable_irq(NVIC_USART1_IRQ);
		usart_enable_rx_interrupt(USART1);
		dwt_delay_ms(250);
		send_command("H100\r\n", 6);
	}
	int_rcvr_params.controlMode = CONTROL_MODE_NONE;
}

uint8_t get_filter_width(uint8_t mod, uint8_t filter){
	if ((mod == 0) || (mod == 1)){
		if ((filter == 0) || (filter == 1)){
			return filter;
		} else {
			return 0;
		}
	} else if (mod == 2) {
		if (filter != 4){
			return filter;
		} else {
			return 0;
		}
	} else if (mod == 3){
		if ((filter == 0) || (filter == 1)){
			return filter;
		} else {
			return 0;
		}
	} else if (mod == 5) {
		if ( (filter == 1) || (filter == 2) || (filter == 3)){
			return filter;
		} else {
			return 1;
		}
	} else if (mod == 6){
		if ((filter == 3) || (filter == 4)){
			return filter;
		} else {
			return 4;
		}
	} else {
		return filter;
	}
}

void change_mod(void){
	if (int_rcvr_params.controlMode == CONTROL_MODE_STANDALONE){
		switch(int_rcvr_params.modulation){
			case 0: //LSB
				int_rcvr_params.modulation = 1; //USB
				break;
			case 1: //USB
				int_rcvr_params.modulation = 2; //AM 
				int_rcvr_params.filter = get_filter_width(2, int_rcvr_params.filter);
				break;
			case 2: //AM
				int_rcvr_params.modulation = 3; //CW
				int_rcvr_params.filter = get_filter_width(3, int_rcvr_params.filter);
				break;
			case 3: //CW
				int_rcvr_params.modulation = 5; //NFM
				int_rcvr_params.filter = get_filter_width(5, int_rcvr_params.filter);
				break;
			case 5: //NFM
				int_rcvr_params.modulation = 6; //WFM
				int_rcvr_params.filter = get_filter_width(6, int_rcvr_params.filter);
				break;
			case 6: //WFM
				int_rcvr_params.modulation = 0; //LSB
				int_rcvr_params.filter = get_filter_width(0, int_rcvr_params.filter);
				break;
			default: break;

		}
		set_reciever_params(); 
	}
}

void change_vol_sql_freq(void){
	if (int_rcvr_params.controlMode == CONTROL_MODE_STANDALONE){
		switch (encoder_fsm){
			case ENCODER_FSM_FREQ:
				encoder_fsm = ENCODER_FSM_VOL;
				rotary_encoder_tim3_set_value(0);
				rotary_encoder_tim3_set_limit(255);
				rotary_encoder_tim3_set_value(int_rcvr_params.volume);	
				lcd_line1_blink_bitmap = 0x0F00;
				break;
			case ENCODER_FSM_VOL:
				encoder_fsm = ENCODER_FSM_SQL;
				rotary_encoder_tim3_set_value(0);
				rotary_encoder_tim3_set_limit(255);
				rotary_encoder_tim3_set_value(int_rcvr_params.squelch_level);	
				lcd_line1_blink_bitmap = 0x00FF;
				break;
			case ENCODER_FSM_SQL:
				encoder_fsm = ENCODER_FSM_FREQ;
				rotary_encoder_tim3_set_value(0);
				rotary_encoder_tim3_set_limit(9);
				rotary_encoder_tim3_set_value(last_encoder);	
				lcd_line1_blink_bitmap = 0x0000;
				break;
			default: break;
		}
	}
}

void change_filter(void){
	if (int_rcvr_params.controlMode == CONTROL_MODE_STANDALONE){
		uint8_t f = int_rcvr_params.filter;
		uint8_t ssb_cw[5] = {1 ,0, 0, 0, 0};
		uint8_t am[5] = {1, 2, 3, 0, 0};
		uint8_t nfm[5] = {1, 2, 3, 1, 1};
		uint8_t wfm[5] = {3, 3, 3, 4, 3};

		switch (int_rcvr_params.modulation){
			case 0: int_rcvr_params.filter = ssb_cw[f]; break;
			case 1: int_rcvr_params.filter = ssb_cw[f]; break;
			case 2: int_rcvr_params.filter = am[f]; break;
			case 3: int_rcvr_params.filter = ssb_cw[f]; break;
			case 5: int_rcvr_params.filter = nfm[f]; break;
			case 6: int_rcvr_params.filter = wfm[f]; break;
			default: break;
		}
		set_reciever_params(); 
	}
}

void set_reciever_params(void){
	char setup_str[] = "K00000000000000000\r\n";
        int2str(int_rcvr_params.frequency, 10, &setup_str[2]);
	setup_str[13] = (char)(int_rcvr_params.modulation + '0'); //12:13
	setup_str[15] = (char)(int_rcvr_params.filter + '0'); //14:15
	send_command(&setup_str[0], 20);
}

void set_volume(void){
	char setup_str[7] = "J4000\r\n";
	int2strhex(int_rcvr_params.volume, 2, &setup_str[3]);
	send_command(&setup_str[0], 7);
}

void set_squelch(void){
	char setup_str[7] = "J4100\r\n";
	int2strhex(int_rcvr_params.squelch_level, 2, &setup_str[3]);
	send_command(&setup_str[0], 7);
}

void switch_agc(void){
	int_rcvr_params.agc_state = int_rcvr_params.agc_state ? 0 : 1;
	if(int_rcvr_params.agc_state){
		send_command("J4501\r\n",7);
	} else {
		send_command("J4500\r\n",7);
	}
}
void switch_att(void){
	int_rcvr_params.attenuator_state = int_rcvr_params.attenuator_state ? 0 : 1;
	if(int_rcvr_params.attenuator_state){
		send_command("J4701\r\n",7);
	} else {
		send_command("J4700\r\n",7);
	}
}
void switch_nb(void){
	int_rcvr_params.noise_blanker_state = int_rcvr_params.noise_blanker_state ? 0 : 1;
	if(int_rcvr_params.noise_blanker_state){
		send_command("J4601\r\n",7);
	} else {
		send_command("J4600\r\n",7);
	}
}

void scan_buttons(void){
	if(!gpio_get(BTN_PORT, BTN_MOD)){
		btn_mod_press = 1;
	} else {
		if(btn_mod_press){
			btn_mod_trg = 1;
			btn_mod_press = 0;
		}
	}
	if(!gpio_get(BTN_PORT, BTN_A)){
		btn_a_press = 1;
	} else {
		if(btn_a_press){
			btn_a_trg = 1;
			btn_a_press = 0;
		}
	}
	if(!gpio_get(BTN_PORT, BTN_B)){
		btn_b_press = 1;
	} else {
		if(btn_b_press){
			btn_b_trg = 1;
			btn_b_press = 0;
		}
	}
	if(!gpio_get(BTN_PORT, BTN_C)){
		btn_c_press = 1;
	} else {
		if(btn_c_press){
			btn_c_trg = 1;
			btn_c_press = 0;
		}
	}
	if(!gpio_get(ROTBTN_PORT, ROTBTN)){
		btn_rot_press = 1;
	} else {
		if(btn_rot_press){
			btn_rot_trg = 1;
			btn_rot_press = 0;
		}
	}
}

void tim4_isr(void)
{
        if (timer_interrupt_source(TIM4, TIM_SR_UIF)) {
                screen_update_flag = 1;
		scan_buttons();
		if (screen_blinker_div == 0){
			screen_blinker_div = BLINKER_DIV;
			screen_blinker_flag ^= 0x01;
		} else {
			screen_blinker_div--;
		}
                /* Clear compare interrupt flag. */
                timer_clear_flag(TIM4, TIM_SR_UIF);
        }
}

void send_command(char* cmd, uint8_t len){
	if (int_rcvr_params.controlMode == CONTROL_MODE_STANDALONE){
		//usart_send_blocking(USART2, '>'); //DBG
		for (uint8_t i = 0; i < len; i++){
			rb_u8_push(&rx_ring_buffer, (uint8_t)cmd[i]);
                	//usart_send_blocking(USART2, cmd[i]); //DBG
		}
		usart_enable_tx_interrupt(USART1); // Immediate send command buffer
	}
}

void usart1_isr(void){
	uint8_t data = 0;
	if (rx_ring_buffer.length > 0) {
		usart_send_blocking(USART2, '@'); //DBG
                while(rb_u8_pop(&rx_ring_buffer, &data)){
			host_cmd_parser(data);
                        usart_send_blocking(USART1, data);
                        usart_send_blocking(USART2, data); //DBG
                }
                usart_disable_tx_interrupt(USART1);
                
        }
	if (usart_get_flag(USART1, USART_SR_RXNE)) {
                data = (uint8_t)usart_recv(USART1);
		radio_reply_parser(data);
                //usart_send_blocking(USART2, data); //DBG
		if (int_rcvr_params.controlMode == CONTROL_MODE_BRIDGE){
                	rb_u8_push(&tx_ring_buffer, data);
			to_cdc_flag = 1;
		}
        }
}

void get_cdc_comm_config(uint32_t speed, uint8_t stop_bits, uint8_t parity, uint8_t data_bits){
	if (int_rcvr_params.controlMode == CONTROL_MODE_STANDALONE){
		const uint32_t stop_bit_map[3] = {USART_STOPBITS_1, USART_STOPBITS_1_5, USART_STOPBITS_2};
		const uint32_t parity_map[5] = {
			USART_PARITY_NONE, 
			USART_PARITY_ODD, 
			USART_PARITY_EVEN, 
			USART_PARITY_NONE, //Not supported "Mark"
			USART_PARITY_NONE, //Not supported "Space"
		};
		usart_disable(USART1);
        	usart_setup(USART1, speed, data_bits, stop_bit_map[stop_bits], USART_MODE_TX_RX, parity_map[parity], USART_FLOWCONTROL_NONE );
		int_rcvr_params.wasInit = ICOM_HAS_CFG;
	}

}
