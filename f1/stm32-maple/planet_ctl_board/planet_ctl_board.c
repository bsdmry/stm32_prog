#include <libopencm3/stm32/rcc.h>
#include  <libopencm3/stm32/gpio.h>
#include  <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "i2c.h"
#include "si5351.h"
#include "dwt.h"
#include "ch455_led_driver.h"
#include "encoder.h"
#include "m62429.h"
#include "planet_ctl_board.h"
#include "morse_buf.h"

volatile uint16_t led_prescaler = LED_REFRESH_PRESCALER;
volatile uint8_t led_update_flag = 0;
volatile uint16_t led_blinker_prescaler = LED_BLINK_PRESCALER;
volatile uint8_t led_blinker_update_flag = 0;
volatile uint16_t tone_prescaler = TONE_PRESCALER;
volatile uint8_t rotary_update_flag = 0;
volatile uint32_t rotary_pos = 0;
volatile uint16_t kbd_prescaler = KEYBOARD_POLLER_PRESCALER;
volatile uint8_t kbd_update_flag = 0;
volatile uint16_t key_prescaler = KEY_POLLER_PRESCALER;
volatile uint8_t key_update_flag = 0;

volatile uint8_t tx_flag = 0;
volatile uint8_t tone_gen = 0;


#define FSM_MAIN_RX 0
#define FSM_RX_AUDIO_VOL_SELECT 1
#define FSM_TX_TONE_VOL_SELECT 2
#define FSM_TX_TONE_FREQ_SELECT 3
#define FSM_TX_OFFSET_SELECT 4
#define FSM_STEP_SELECT 5
#define FSM_CW_SPEED_SELECT 6
#define FSM_RX_AUDIO_VOL_CHG 7
#define FSM_TX_TONE_VOL_CHG 8
#define FSM_TX_TONE_FREQ_CHG 9
#define FSM_TX_OFFSET_CHG 10
#define FSM_STEP_CHG 11
#define FSM_CW_SPEED_CHG 12
#define FSM_TX 13
volatile uint8_t ui_fsm_state = FSM_MAIN_RX;

#define MAIN_KHZ_MAX 200
#define MAIN_HHZ_MAX 2000
#define MENU_MAX 5
#define AUDIO_VOL_MAX 100
#define TONE_VOL_MAX 100
#define TONE_FREQ_MAX 1000
#define OFFSET_MAX 1000
#define STEP_MAX 1
#define CW_SPEED_MAX 3

uint8_t btn0_trg = 0;
uint8_t btn0_press = 0;
volatile uint8_t btn1_trg = 0;
volatile uint8_t btn1_press = 0;
uint8_t dash_press = 0;
uint8_t dot_press = 0;

ch455* display;
m62429* snd;
planet radioCfg;

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
}

void display_setup(void) {
	gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_10_MHZ,  GPIO_CNF_OUTPUT_PUSHPULL, RX_LED_PIN);
	gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_10_MHZ,  GPIO_CNF_OUTPUT_PUSHPULL, TX_LED_PIN);
	gpio_set(LED_PORT, RX_LED_PIN);
	gpio_clear(LED_PORT, TX_LED_PIN);

	display = ch455_init(GPIOB, GPIO11, GPIO10);
	ch455_set_brightness(display, 4);
	ch455_write_cfg(display);

	//ch455_set_flashing_bitset(display, 0b00001100);
	//ch455_send_text(display, "Ab", 2, 0);
	//ch455_send_number(display, 56, 0, 2, 0);
}

void radio_init_cfg(void){
	radioCfg.frequency = 7000000;
	radioCfg.kHz = 0;
	radioCfg.hHz = 0;
	radioCfg.rxVolLevel = 50;
	radioCfg.toneVolLevel = 20;
	radioCfg.cwSpeeed = 1;
	radioCfg.toneFreq = 500;
	radioCfg.txOffset = 100;
	radioCfg.currentMenuEntity = 0;
	radioCfg.stepSize = 0; //0 - 1 kiloherz, 1 - 1 hectoherz
	radioCfg.freqDisplayMode = 0; // Freq:7005.1 Mhz. 0 - on display 7005, 1 - on display 005.1
}

void sound_setup(void){
	gpio_set_mode(TONE_PORT, GPIO_MODE_OUTPUT_10_MHZ,  GPIO_CNF_OUTPUT_PUSHPULL, TONE_PIN);
	snd = m62429_init(GPIOB, GPIO15, GPIO13);
	m62429_set_volume(snd, M62429_CH1, 0);
	dwt_delay_ms(20);
	m62429_set_volume(snd, M62429_CH2, 0);
}

void controls_setup(void){
	rotary_encoder_tim1_setup(MAIN_KHZ_MAX);
	gpio_set_mode(BTN0_PORT, GPIO_MODE_INPUT,  GPIO_CNF_INPUT_FLOAT, BTN0_PIN);
	gpio_set_mode(BTN1_PORT, GPIO_MODE_INPUT,  GPIO_CNF_INPUT_FLOAT, BTN1_PIN);
	gpio_set_mode(KEY_PORT, GPIO_MODE_INPUT,  GPIO_CNF_INPUT_FLOAT, KEY_DAH_PIN);
	gpio_set_mode(KEY_PORT, GPIO_MODE_INPUT,  GPIO_CNF_INPUT_FLOAT, KEY_DIT_PIN);
}
void rf_setup(void){
	gpio_set_mode(RELAY_PORT, GPIO_MODE_OUTPUT_10_MHZ,  GPIO_CNF_OUTPUT_PUSHPULL, RELAY_PIN);
	gpio_clear(RELAY_PORT, RELAY_PIN);
	i2c_1_setup();
        si5351_init(I2C1, 0);
}

int main(void)
{
	clock_setup();
	dwt_setup();
	tim3_init();
	tim2_init();
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_10_MHZ,  GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
	gpio_set(GPIOC, GPIO13);
	sound_setup();
	dwt_delay_ms(1000);
	display_setup();
	controls_setup();
	radio_init_cfg();
	rf_setup();
	rf_switch2rx();
	audio_switch2rx();
	show_main_screen();

	while(1){
		if (led_update_flag){
			led_update_flag = 0;
			ch455_display_data(display);
		}
		if (led_blinker_update_flag){
			led_blinker_update_flag = 0;
			display->blinkSwitch ^= 1;
		}
		if (rotary_update_flag){
			rotary_update_flag = 0;
			rotary_action();
		}
		if (btn0_trg){
			btn0_trg = 0;
			btn0_action();
		}
		if (btn1_trg){
			btn1_trg = 0;
			btn1_action();
		}
		if((morse_buf.length > 0) && (tx_flag == 0 )){
			enable_tx();
		}
	}
}

void enable_tx(void){
	gpio_clear(GPIOC, GPIO13);// DBG
	tx_flag = 1;
	gpio_set(RELAY_PORT, RELAY_PIN);
	gpio_clear(LED_PORT, RX_LED_PIN);
	gpio_set(LED_PORT, TX_LED_PIN);
	uint8_t data = 0;
	pop(&morse_buf, &data);
	setup_cw_len(data);
	audio_switch2tx();
	rf_switch2tx();
	timer_enable_counter(TIM2);
}

void disable_tx(void){
	tx_flag = 0;
	gpio_clear(RELAY_PORT, RELAY_PIN);
	gpio_set(LED_PORT, RX_LED_PIN);
	gpio_clear(LED_PORT, TX_LED_PIN);
	timer_disable_counter(TIM2);
	rf_switch2rx();
	audio_switch2rx();
}

void set_encoder(uint32_t value, uint32_t max_value){
	rotary_encoder_tim1_set_limit(max_value);
	rotary_encoder_tim1_set_value(value);
}

void audio_switch2rx(void){
	tone_gen = 0;
	m62429_set_volume(snd, M62429_CH2, 0);
	m62429_set_volume(snd, M62429_CH1,(uint8_t)radioCfg.rxVolLevel);
}

void audio_switch2tx(void){
	tone_gen = 1;
	m62429_set_volume(snd, M62429_CH1, 0);
	m62429_set_volume(snd, M62429_CH2, (uint8_t)radioCfg.toneVolLevel);
}

void rf_switch2rx(void){
	si5351_setup_clk1(I2C1, (int32_t)(radioCfg.frequency - radioCfg.txOffset + 10700000), SI5351_DRIVE_STRENGTH_2MA);
	si5351_setup_clk0(I2C1, 10700000, SI5351_DRIVE_STRENGTH_2MA);
	//si5351_enable_outputs(I2C1, SI5351_OUT_CLK_1);
	si5351_enable_outputs(I2C1, SI5351_OUT_CLK_0 | SI5351_OUT_CLK_1);

}
void rf_switch2tx(void){
	si5351_setup_clk2(I2C1, (int32_t)(radioCfg.frequency), SI5351_DRIVE_STRENGTH_8MA);
	si5351_enable_outputs(I2C1, SI5351_OUT_CLK_2);
}

void show_menu_rx_audio_vol(void) { ui_fsm_state = FSM_RX_AUDIO_VOL_SELECT; set_encoder(0, MENU_MAX); ch455_send_text(display, "rc.LE", 0, 0);}
void show_menu_tx_tone_vol(void) { ui_fsm_state = FSM_TX_TONE_VOL_SELECT; ch455_send_text(display, "to.LE", 0, 0);}
void show_menu_tx_tone_freq(void) { ui_fsm_state = FSM_TX_TONE_FREQ_SELECT; ch455_send_text(display, "to.Fq", 0, 0);}
void show_menu_tx_offset(void) { ui_fsm_state = FSM_TX_OFFSET_SELECT; ch455_send_text(display, "OFSt", 0, 0);}
void show_menu_step_select(void) { ui_fsm_state = FSM_STEP_SELECT; ch455_send_text(display, "StEP", 0, 0);}
void show_menu_cw_speed(void) { ui_fsm_state = FSM_CW_SPEED_SELECT; ch455_send_text(display, "t.SPd", 0, 0);}


void show_main_screen(void){
	ui_fsm_state = FSM_MAIN_RX;
	if (radioCfg.stepSize == 0) {
		set_encoder((uint32_t)radioCfg.kHz, MAIN_KHZ_MAX);
	} else {
		set_encoder((uint32_t)((radioCfg.kHz * 10) + radioCfg.hHz), MAIN_HHZ_MAX);
	}
	display_frequency();
}

void display_frequency(void){
	if (radioCfg.freqDisplayMode == 0){
		ch455_send_number(display, (radioCfg.kHz + 7000), 0, 4, 0);
	} else {
		ch455_send_fraction(display, radioCfg.kHz, 1, 3, radioCfg.hHz, 0, 1, 0);
	}
}

void update_frequency(uint32_t value){
	if (radioCfg.stepSize == 0) {
		radioCfg.kHz = (uint16_t)value;
	} else {
		radioCfg.kHz = (uint16_t)(value/10);
		radioCfg.hHz = (uint16_t)(value%10);
	}
	radioCfg.frequency = 7000000 + (radioCfg.kHz * 1000) + (radioCfg.hHz * 100);
	rf_switch2rx();
	display_frequency();
}

void show_edit_rx_audio_vol(void){ 
	ui_fsm_state = FSM_RX_AUDIO_VOL_CHG; 
	ch455_send_number(display, (uint16_t)(radioCfg.rxVolLevel), 0, 4, 0); 
	set_encoder(radioCfg.rxVolLevel, AUDIO_VOL_MAX); 
}
void show_edit_tx_tone_vol(void){ 
	ui_fsm_state = FSM_TX_TONE_VOL_CHG; 
	audio_switch2tx();
	ch455_send_number(display, (uint16_t)(radioCfg.toneVolLevel), 0, 4, 0); 
	set_encoder(radioCfg.toneVolLevel, TONE_VOL_MAX);
}
void show_edit_tx_tone_freq(void){ 
	ui_fsm_state = FSM_TX_TONE_FREQ_CHG; 
	audio_switch2tx();
	ch455_send_number(display, (uint16_t)(radioCfg.toneFreq), 0, 4, 0); 
	set_encoder(radioCfg.toneFreq, TONE_FREQ_MAX);
}
void show_edit_tx_offset(void){ 
	ui_fsm_state = FSM_TX_OFFSET_CHG; 
	ch455_send_number(display, (uint16_t)(radioCfg.txOffset), 0, 4, 0); 
	set_encoder(radioCfg.txOffset, OFFSET_MAX); 
}
void show_edit_step(void){ 
	ui_fsm_state = FSM_STEP_CHG; 
	ch455_send_number(display, (uint16_t)(radioCfg.stepSize), 0, 4, 0); 
	set_encoder(radioCfg.stepSize, STEP_MAX); 
}
void show_edit_cw_speed(void){ 
	ui_fsm_state = FSM_CW_SPEED_CHG; 
	ch455_send_number(display, (uint16_t)(radioCfg.cwSpeeed), 0, 4, 0); 
	set_encoder(radioCfg.cwSpeeed, CW_SPEED_MAX); 
}

void show_update_rx_audio_vol(uint32_t value){
	radioCfg.rxVolLevel = (uint16_t)value;
	m62429_set_volume(snd, M62429_CH1, (uint8_t)value);
	ch455_send_number(display, (uint16_t)(radioCfg.rxVolLevel), 0, 4, 0);
}
void show_update_tx_tone_vol(uint32_t value){
	radioCfg.toneVolLevel = (uint16_t)value;
	m62429_set_volume(snd, M62429_CH2, (uint8_t)value);
	ch455_send_number(display, (uint16_t)(radioCfg.toneVolLevel), 0, 4, 0);
}
void show_update_tx_tone_freq(uint32_t value){
	radioCfg.toneFreq = (uint16_t)value;
	ch455_send_number(display, (uint16_t)(radioCfg.toneFreq), 0, 4, 0);
}
void show_update_tx_offset(uint32_t value){
	radioCfg.txOffset = (uint16_t)value;
	ch455_send_number(display, (uint16_t)(radioCfg.txOffset), 0, 4, 0);
	rf_switch2rx();
}
void show_update_step(uint32_t value){
	radioCfg.stepSize = (uint16_t)value;
	ch455_send_number(display, (uint16_t)(radioCfg.stepSize), 0, 4, 0);
}
void show_update_cw_speed(uint32_t value){
	radioCfg.cwSpeeed = (uint16_t)value;
	ch455_send_number(display, (uint16_t)(radioCfg.cwSpeeed), 0, 4, 0);
}


void btn0_action(void){ //config-enter
	switch (ui_fsm_state){
		case FSM_MAIN_RX: show_menu_rx_audio_vol(); break;
		case FSM_RX_AUDIO_VOL_SELECT: radioCfg.currentMenuEntity = 0; show_edit_rx_audio_vol(); break;
		case FSM_TX_TONE_VOL_SELECT: radioCfg.currentMenuEntity = 1; show_edit_tx_tone_vol(); break;
		case FSM_TX_TONE_FREQ_SELECT: radioCfg.currentMenuEntity = 2; show_edit_tx_tone_freq(); break;
		case FSM_TX_OFFSET_SELECT: radioCfg.currentMenuEntity = 3; show_edit_tx_offset(); break;
		case FSM_STEP_SELECT: radioCfg.currentMenuEntity = 4; show_edit_step(); break;
		case FSM_CW_SPEED_SELECT: radioCfg.currentMenuEntity = 5; show_edit_cw_speed(); break;
		default: break;
	}
}

void btn1_action(void){//exit-chg freq view
	switch (ui_fsm_state){
		case FSM_RX_AUDIO_VOL_CHG: show_menu_rx_audio_vol(); break;
		case FSM_TX_TONE_VOL_CHG: set_encoder(radioCfg.currentMenuEntity, MENU_MAX); show_menu_tx_tone_vol(); audio_switch2rx(); break;
		case FSM_TX_TONE_FREQ_CHG: set_encoder(radioCfg.currentMenuEntity, MENU_MAX); show_menu_tx_tone_freq(); audio_switch2rx();  break;
		case FSM_TX_OFFSET_CHG: set_encoder(radioCfg.currentMenuEntity, MENU_MAX); show_menu_tx_offset(); break;
		case FSM_STEP_CHG: set_encoder(radioCfg.currentMenuEntity, MENU_MAX); show_menu_step_select();  break;
		case FSM_CW_SPEED_CHG: set_encoder(radioCfg.currentMenuEntity, MENU_MAX); show_menu_cw_speed(); break;
		case FSM_MAIN_RX: radioCfg.freqDisplayMode ^= 1; display_frequency(); break;
		default: show_main_screen();  break;
	}
}

void roll_over_menu(uint16_t position){
	switch(position){
		case 0: show_menu_rx_audio_vol(); break;
		case 1: show_menu_tx_tone_vol(); break;
		case 2: show_menu_tx_tone_freq(); break;
		case 3: show_menu_tx_offset(); break;
		case 4: show_menu_step_select(); break;
		case 5: show_menu_cw_speed(); break;
		default: break;
	}
}

void rotary_action(void){
	switch (ui_fsm_state){
		case FSM_MAIN_RX: update_frequency(rotary_pos); break;
		case FSM_RX_AUDIO_VOL_SELECT: roll_over_menu(rotary_pos); break;
		case FSM_TX_TONE_VOL_SELECT: roll_over_menu(rotary_pos); break;
		case FSM_TX_TONE_FREQ_SELECT: roll_over_menu(rotary_pos); break;
		case FSM_TX_OFFSET_SELECT: roll_over_menu(rotary_pos); break;
		case FSM_STEP_SELECT: roll_over_menu(rotary_pos); break;
		case FSM_CW_SPEED_SELECT: roll_over_menu(rotary_pos); break;
		case FSM_RX_AUDIO_VOL_CHG: show_update_rx_audio_vol(rotary_pos); break;
		case FSM_TX_TONE_VOL_CHG: show_update_tx_tone_vol(rotary_pos); break;
		case FSM_TX_TONE_FREQ_CHG: show_update_tx_tone_freq(rotary_pos); break;
		case FSM_TX_OFFSET_CHG: show_update_tx_offset(rotary_pos); break;
		case FSM_STEP_CHG: show_update_step(rotary_pos); break;
		case FSM_CW_SPEED_CHG: show_update_cw_speed(rotary_pos); break;
		default: break;
	}
}

void tim3_init(void){
    rcc_periph_clock_enable(RCC_TIM3);
    rcc_periph_reset_pulse(RST_TIM3);
    timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

    timer_continuous_mode(TIM3);
    timer_set_prescaler(TIM3, 36-1);
    timer_set_period(TIM3, 100-1);
    timer_disable_preload(TIM3);
    timer_enable_irq(TIM3, TIM_DIER_UIE);
    timer_enable_counter(TIM3);
    nvic_enable_irq(NVIC_TIM3_IRQ);
}

void tim2_init(void){
    rcc_periph_clock_enable(RCC_TIM2);
    rcc_periph_reset_pulse(RST_TIM2);
    timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    timer_continuous_mode(TIM2);
    timer_set_prescaler(TIM2, 36000-1); //72 Mhz / 36000 = 2 kHz = 0.5 ms
    timer_set_period(TIM2, 1200-1); // 2 Mhz / 20000 = 100 Hz

    timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_FROZEN);
    timer_set_oc_value(TIM2, TIM_OC1, 600-1);
    timer_enable_oc_output(TIM2, TIM_OC1);

    timer_set_counter(TIM2, 0);
    timer_enable_preload(TIM2);
    timer_enable_irq(TIM2, TIM_DIER_UIE | TIM_DIER_CC1IE );
    nvic_enable_irq(NVIC_TIM2_IRQ);
}

void setup_cw_len(uint8_t signal){
	//dot - 0.3 sec; dash - 0.9 sec
	uint16_t dot = 200; 
	//uint16_t dot = 800; 
	uint32_t period, oc;
	if (signal){ //Dash
		period = dot * 4;
		oc = dot *3;
	} else { //Dot
		period = dot * 2;
		oc = dot;
	}
    	timer_set_period(TIM2, period-1);
    	timer_set_oc_value(TIM2, TIM_OC1, oc-1);
}

void tim2_isr(void){
	if (timer_interrupt_source(TIM2, TIM_SR_UIF)) {
        	timer_clear_flag(TIM2, TIM_SR_UIF);
		uint8_t data = 0;
		if (pop(&morse_buf, &data)){
			gpio_clear(GPIOC, GPIO13);// DBG
			setup_cw_len(data); 
			tone_gen = 1;
			si5351_enable_outputs(I2C1, SI5351_OUT_CLK_2);
			//m62429_set_volume(snd, M62429_CH2, (uint8_t)radioCfg.toneVolLevel);
		} else {
			disable_tx();
		}
	}
	if (timer_interrupt_source(TIM2, TIM_SR_CC1IF)) {
        	timer_clear_flag(TIM2, TIM_SR_CC1IF);
		gpio_set(GPIOC, GPIO13);// DBG
		tone_gen = 0;
		si5351_disable_all_outputs(I2C1);
		//m62429_set_volume(snd, M62429_CH2, 0);	
	}
	

}

void tim3_isr(void){
	if (timer_interrupt_source(TIM3, TIM_SR_UIF)) {
        	timer_clear_flag(TIM3, TIM_SR_UIF);
		kbd_prescaler--;
		if (kbd_prescaler == 0){
			kbd_prescaler = KEYBOARD_POLLER_PRESCALER;

			/*if(!gpio_get(BTN1_PORT, BTN1_PIN)){
				if (btn1_press){
					btn1_press = 0;
					btn1_trg = 1;
				} else {
					btn1_press = 1;
				}
			} else {
				btn1_press = 0;
			}*/
			if(!gpio_get(BTN1_PORT, BTN1_PIN)){
				btn1_press = 1;
			} else {
				if(btn1_press){
					btn1_trg = 1;
					btn1_press = 0;
				}
			}

			if(!gpio_get(BTN0_PORT, BTN0_PIN)){
				btn0_press = 1;
			} else {
				if(btn0_press){
					btn0_trg = 1;
					btn0_press = 0;
				}
			}

		}

		key_prescaler--;
		if (key_prescaler == 0){
			key_prescaler = KEY_POLLER_PRESCALER;
			if(!gpio_get(KEY_PORT, KEY_DAH_PIN)){
				dash_press = 1;
			} else {
				if(dash_press){
					push(&morse_buf, 0x01);
					dash_press = 0;
				}

			}

			if(!gpio_get(KEY_PORT, KEY_DIT_PIN)){
				dot_press = 1;
			} else {
				if(dot_press){
					push(&morse_buf, 0x00);
					dot_press = 0;
				}
			}
		}


		if (rotary_pos != rotary_encoder_tim1_get_value()){
			rotary_pos = rotary_encoder_tim1_get_value();
			rotary_update_flag = 1;
		}
		tone_prescaler--;
		if (tone_prescaler == 0){
			if (tone_gen){
				gpio_toggle(TONE_PORT, TONE_PIN);
			}
			tone_prescaler = TONE_PRESCALER;
		}
		led_prescaler--;
		if (led_prescaler == 0){
			led_prescaler = LED_REFRESH_PRESCALER;
			led_update_flag = 1;
		}
		led_blinker_prescaler--;
		if (led_blinker_prescaler == 0){
			led_blinker_prescaler = LED_BLINK_PRESCALER;
			led_blinker_update_flag = 1;
		}
	}
}
