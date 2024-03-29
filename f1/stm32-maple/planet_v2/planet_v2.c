#include <libopencm3/stm32/rcc.h>
#include  <libopencm3/stm32/gpio.h>
#include  <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/cm3/nvic.h>
#include "i2c.h"
#include "winstar_lcd.h"
#include "si5351.h"
#include "dwt.h"
#include "flash_mem.h"
#include "encoder.h"
#include "m62429.h"
#include "planet_v2.h"
#include "morse_buf.h"

volatile uint16_t tone_prescaler = TONE_PRESCALER;
volatile uint8_t rotary_update_flag = 0;
volatile uint32_t rotary_pos = 0;
volatile uint16_t kbd_prescaler = KEYBOARD_POLLER_PRESCALER;
volatile uint8_t kbd_update_flag = 0;
volatile uint16_t key_prescaler = KEY_POLLER_PRESCALER;
volatile uint8_t key_update_flag = 0;
volatile uint16_t main_screen_update_prescaler = SCREEN_REFRESH_PRESCALER;
volatile uint8_t main_screen_update_flag = 0;

volatile uint8_t tx_flag = 0;
volatile uint8_t tone_gen = 0;

volatile uint8_t dot_gen = 0;
volatile uint8_t dash_gen = 0;

#define CFG_ID_RX_VOL 3
#define CFG_ID_TONE_VOL 4
#define CFG_ID_CW_SPD 5
#define CFG_ID_TONE_FREQ 6
#define CFG_ID_TX_OFFSET 7
#define CFG_ID_STP_SIZE 8
#define CFG_ID_BACKLIGHT 9

#define FSM_MAIN_RX 0

#define FSM_RX_AUDIO_VOL_SELECT 1
#define FSM_TX_TONE_VOL_SELECT 2
#define FSM_TX_TONE_FREQ_SELECT 3
#define FSM_TX_OFFSET_SELECT 4
#define FSM_STEP_SELECT 5
#define FSM_CW_SPEED_SELECT 6
#define FSM_BACKLIGHT_SELECT 7

#define FSM_RX_AUDIO_VOL_CHG 8
#define FSM_TX_TONE_VOL_CHG 9
#define FSM_TX_TONE_FREQ_CHG 10
#define FSM_TX_OFFSET_CHG 11
#define FSM_STEP_CHG 12
#define FSM_CW_SPEED_CHG 13
#define FSM_BACKLIGHT_CHG 14

#define FSM_TX 15

#define MENU_MAX 6 //Max menu index (from zero)

volatile uint8_t ui_fsm_state = FSM_MAIN_RX;

#define AUDIO_VOL_MAX 100
#define TONE_VOL_MAX 100
#define TONE_FREQ_MAX 1000
#define OFFSET_MAX 1000
#define STEP_MAX 1
#define CW_SPEED_MAX 3
#define MAIN_KHZ_MAX 200
#define MAIN_HHZ_MAX 2000
#define BACKLIGHT_MAX 1

uint8_t btn0_trg = 0;
uint8_t btn0_press = 0;
volatile uint8_t btn1_trg = 0;
volatile uint8_t btn1_press = 0;
volatile uint8_t btn2_trg = 0;
volatile uint8_t btn2_press = 0;
uint8_t dash_press = 0;
uint8_t dot_press = 0;

m62429* snd;
planet_v2 radioCfg;
winstar_lcd* display;
flash_storage *fs;

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
}


int main(void)
{
	clock_setup();
	dwt_setup();
	dwt_delay_ms(1000);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ,  GPIO_CNF_OUTPUT_PUSHPULL, GPIO0);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ,  GPIO_CNF_OUTPUT_PUSHPULL, GPIO1);
	gpio_clear(GPIOA, GPIO0);
	gpio_clear(GPIOA, GPIO1);
	adc_setup();
	tim3_init();
	tim2_init();
	sound_setup();
	display_setup();
	controls_setup();
	radio_init_cfg();
	rotary_encoder_tim1_setup(MAIN_KHZ_MAX);
	rf_setup();
	audio_switch2rx();
	rf_switch2rx();
	show_main_screen();
	while(1){
		if (rotary_update_flag){
			rotary_update_flag = 0;
			rotary_action();
		}	
		if (main_screen_update_flag){
			main_screen_update_flag = 0;
			display_frequency();
		} 
		if (btn0_trg){
			btn0_trg = 0;
			btn0_action();
		}
		if (btn1_trg){
			btn1_trg = 0;
			btn1_action();
		}
		if (btn2_trg){
			btn2_trg = 0;
			btn2_action();
		}
		/*if((morse_buf.length > 0) && (tx_flag == 0 )){
			enable_tx();
		}*/
		if((dot_gen || dash_gen) && (tx_flag == 0 )){
			enable_tx();
		}
	}
}

void save_cfg_on_flash(void){
	uint32_t rp = 0;
	flash_memory_storage_erase(fs);
	flash_memory_config_container_save(fs);
        dwt_delay_ms(50);
	rp = radioCfg.rxVolLevel;
	flash_memory_write_option(fs, CFG_ID_RX_VOL, 1, &rp);
	rp = radioCfg.toneVolLevel;
	flash_memory_write_option(fs, CFG_ID_TONE_VOL, 1, &rp);
	rp = radioCfg.cwSpeed;
	flash_memory_write_option(fs, CFG_ID_CW_SPD, 1, &rp);
	rp = radioCfg.toneFreq;
	flash_memory_write_option(fs, CFG_ID_TONE_FREQ, 1, &rp);
	rp = radioCfg.txOffset;
	flash_memory_write_option(fs, CFG_ID_TX_OFFSET, 1, &rp);
	rp = radioCfg.stepSize;
	flash_memory_write_option(fs, CFG_ID_STP_SIZE, 1, &rp);
	rp = display->backlight;
	flash_memory_write_option(fs, CFG_ID_BACKLIGHT, 1, &rp);

}

void restore_cfg_from_flash(void){
	uint32_t rp = 0;
	flash_memory_read_option(fs, CFG_ID_RX_VOL, &rp);
	radioCfg.rxVolLevel = (uint16_t)rp;
	flash_memory_read_option(fs, CFG_ID_TONE_VOL, &rp);
	radioCfg.toneVolLevel = (uint16_t)rp;
	flash_memory_read_option(fs, CFG_ID_CW_SPD, &rp);
	radioCfg.cwSpeed = (uint8_t)rp;
	flash_memory_read_option(fs, CFG_ID_TONE_FREQ, &rp);
	radioCfg.toneFreq = (uint16_t)rp;
	flash_memory_read_option(fs, CFG_ID_TX_OFFSET, &rp);
	radioCfg.txOffset = (uint16_t)rp;
	flash_memory_read_option(fs, CFG_ID_STP_SIZE, &rp);
	radioCfg.stepSize = (uint8_t)rp;
	flash_memory_read_option(fs, CFG_ID_BACKLIGHT, &rp);
	display->backlight = (uint8_t)rp;

}

void radio_init_cfg(void){
	fs = flash_memory_storage_init(FLASH_MAX_ADDRES_F103C8, 1, FLASH_PAGE_SIZE_MEDIUM);
	uint8_t cfg_read_result = flash_memory_config_container_restore(fs);
	uint8_t reinit_flash_flag = 0;

	if(!gpio_get(BTN0_PORT, BTN0_PIN)){ //Encoder button pressed on startup
		reinit_flash_flag = 1;
	} else {
		if (cfg_read_result || (fs->options_count != 7)){ //No config detected. Create a new default
			reinit_flash_flag = 1;
		}
	}
	
	if (reinit_flash_flag){
		radioCfg.frequency = 7000000;
		radioCfg.kHz = 0;
		radioCfg.hHz = 0;
		radioCfg.rxVolLevel = 50;
		radioCfg.toneVolLevel = 20;
		radioCfg.cwSpeed = 1;
		radioCfg.toneFreq = 500;
		radioCfg.txOffset = 100;
		radioCfg.currentMenuEntity = 0;
		radioCfg.stepSize = 0; //0 - 1 kiloherz, 1 - 1 hectoherz
		option_record recs[7] = { 
			{.record_id = CFG_ID_RX_VOL, .len = 1, .address = 0},
	       		{.record_id = CFG_ID_TONE_VOL, .len = 1, .address = 0},
	       		{.record_id = CFG_ID_CW_SPD, .len = 1, .address = 0},
	       		{.record_id = CFG_ID_TONE_FREQ, .len = 1, .address = 0},
	       		{.record_id = CFG_ID_TX_OFFSET, .len = 1, .address = 0},
	       		{.record_id = CFG_ID_STP_SIZE, .len = 1, .address = 0},
	       		{.record_id = CFG_ID_BACKLIGHT, .len = 1, .address = 0},
		};
		fs->options_count = 7;
		fs->options = &recs[0];
		save_cfg_on_flash();
	} else {
		restore_cfg_from_flash();
		radioCfg.frequency = 7000000;
		radioCfg.kHz = 0;
		radioCfg.hHz = 0;
		radioCfg.currentMenuEntity = 0;
	}

}

void controls_setup(void){
	gpio_set_mode(BTN0_PORT, GPIO_MODE_INPUT,  GPIO_CNF_INPUT_FLOAT, BTN0_PIN);
	gpio_set_mode(BTN1_PORT, GPIO_MODE_INPUT,  GPIO_CNF_INPUT_FLOAT, BTN1_PIN);
	gpio_set_mode(BTN2_PORT, GPIO_MODE_INPUT,  GPIO_CNF_INPUT_FLOAT, BTN2_PIN);
	gpio_set_mode(KEY_PORT, GPIO_MODE_INPUT,  GPIO_CNF_INPUT_FLOAT, KEY_DAH_PIN);
	gpio_set_mode(KEY_PORT, GPIO_MODE_INPUT,  GPIO_CNF_INPUT_FLOAT, KEY_DIT_PIN);
}

void adc_setup(void){
	rcc_periph_clock_enable(RCC_ADC1);
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO6);
	adc_power_off(ADC1);
	adc_disable_scan_mode(ADC1);
	adc_set_single_conversion_mode(ADC1);
	adc_disable_external_trigger_regular(ADC1);
	adc_set_right_aligned(ADC1);
	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_28DOT5CYC);
	adc_power_on(ADC1);
	for (uint32_t i = 0; i < 800000; i++){
		__asm__("nop");
	}
	adc_reset_calibration(ADC1);
	adc_calibrate(ADC1);
}


void display_setup(void) {
	i2c_1_setup();
	display = winstar_init(WINSTAR_INTERFACE_I2C, I2C1, WINSTAR_PCF8574_ADDR);
	display->backlight = 1;
	gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_10_MHZ,  GPIO_CNF_OUTPUT_PUSHPULL, RX_LED_PIN);
	gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_10_MHZ,  GPIO_CNF_OUTPUT_PUSHPULL, TX_LED_PIN);
	gpio_set(LED_PORT, RX_LED_PIN);
	gpio_clear(LED_PORT, TX_LED_PIN);
}

void sound_setup(void){
	gpio_set_mode(TONE_PORT, GPIO_MODE_OUTPUT_10_MHZ,  GPIO_CNF_OUTPUT_PUSHPULL, TONE_PIN);
	snd = m62429_init(SND_MIX_PORT, SND_MIX_DATA_PIN, SND_MIX_CLK_PIN);
	m62429_set_volume(snd, M62429_CH1, 0);
	dwt_delay_ms(20);
	m62429_set_volume(snd, M62429_CH2, 0);
}

void rf_setup(void){
	gpio_set_mode(RELAY_PORT, GPIO_MODE_OUTPUT_10_MHZ,  GPIO_CNF_OUTPUT_PUSHPULL, RELAY_PIN);
	gpio_clear(RELAY_PORT, RELAY_PIN);
	i2c_2_setup();
        si5351_init(I2C2, 0);
}

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
	if (ui_fsm_state == FSM_MAIN_RX){
        	winstar_display_numeric(display, radioCfg.kHz + 7000, 3,  1, 0);
        	winstar_display(display, ".", 1, 4);
        	winstar_display_numeric(display, radioCfg.hHz, 0, 1, 5);
        	winstar_display(display, "  ", 1, 6);
		char charge_info[8];
		uint32_t volts = read_adc();
		build_charge_string(volts, charge_info);
        	winstar_display(display, charge_info, 2, 0);
        	//winstar_display_numeric(display, volts, 7,  2, 0);
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

void set_encoder(uint32_t value, uint32_t max_value){
	rotary_encoder_tim1_set_limit(max_value);
	rotary_encoder_tim1_set_value(value);
}

uint32_t read_adc(void){
	uint8_t channel_array[16];
	channel_array[0] = 6;
	adc_set_regular_sequence(ADC1, 1, channel_array);
	adc_start_conversion_direct(ADC1);
	while (!(adc_eoc(ADC1)));
	uint32_t val = adc_read_regular(ADC1);
	float volt = (float)val * 0.03607734; //this coefficent depends on a position of a voltage divider connected to a power line
	return (uint32_t)volt;
}

void build_charge_string(uint32_t voltage, char *str){
	uint8_t dig;
	memset(str, 0x20, 8);
	str[0] = 'B';
	dig = voltage % 10;
	str[5] = (char)(dig+48);
	str[4] = '.';
	voltage /= 10;
	dig = voltage % 10;
	str[3] = (char)(dig+48);
	voltage /= 10;
	dig = voltage % 10;
	str[2] = (char)(dig+48);
	str[6] = 'V';
}

void enable_tx(void){
	tx_flag = 1;
	gpio_set(RELAY_PORT, RELAY_PIN);
	gpio_clear(LED_PORT, RX_LED_PIN);
	gpio_set(LED_PORT, TX_LED_PIN);
	/*uint8_t data = 0;
	pop(&morse_buf, &data);
	setup_cw_len(data);*/
	if (dash_gen){
		setup_cw_len(1);
	} else if (dot_gen){
		setup_cw_len(0);
	}
	audio_switch2tx();
	rf_switch2tx();
	timer_set_counter(TIM2, 0);
	timer_enable_counter(TIM2);
}

void disable_tx(void){
	tx_flag = 0;
	gpio_clear(RELAY_PORT, RELAY_PIN);
	gpio_set(LED_PORT, RX_LED_PIN);
	gpio_clear(LED_PORT, TX_LED_PIN);
	timer_disable_counter(TIM2);
	timer_set_counter(TIM2, 0);
	rf_switch2rx();
	audio_switch2rx();
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
	/*uint32_t if_freq = 10700000;
	uint32_t crystal_correction = 1000;
	si5351_setup_clk1(I2C2, (int32_t)(radioCfg.frequency - crystal_correction - radioCfg.txOffset + if_freq), SI5351_DRIVE_STRENGTH_2MA);
	si5351_setup_clk2(I2C2, if_freq, SI5351_DRIVE_STRENGTH_2MA);
	si5351_enable_outputs(I2C2, SI5351_OUT_CLK_1 | SI5351_OUT_CLK_2);*/
	
	uint32_t crystal_correction = 2000;
	uint32_t if_freq = 10700000;
	si5351_setup_clk1(I2C2, (int32_t)(radioCfg.frequency + (if_freq - crystal_correction)), SI5351_DRIVE_STRENGTH_2MA);
	si5351_setup_clk2(I2C2, (uint32_t)(if_freq + radioCfg.txOffset) , SI5351_DRIVE_STRENGTH_2MA);
	si5351_enable_outputs(I2C2, SI5351_OUT_CLK_1 | SI5351_OUT_CLK_2);

}
void rf_switch2tx(void){
	si5351_setup_clk0(I2C2, (int32_t)(radioCfg.frequency), SI5351_DRIVE_STRENGTH_8MA);
	si5351_enable_outputs(I2C2, SI5351_OUT_CLK_0);
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

void tim3_isr(void){
	if (timer_interrupt_source(TIM3, TIM_SR_UIF)) {
        	timer_clear_flag(TIM3, TIM_SR_UIF);
		kbd_prescaler--;
		if (kbd_prescaler == 0){
			kbd_prescaler = KEYBOARD_POLLER_PRESCALER;

			if(!gpio_get(BTN2_PORT, BTN2_PIN)){
				btn2_press = 1;
			} else {
				if(btn2_press){
					btn2_trg = 1;
					btn2_press = 0;
				}
			}

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
		/*CW key*/
		key_prescaler--;
		if (key_prescaler == 0){
			key_prescaler = KEY_POLLER_PRESCALER;
			/* Buf handling
			if(!gpio_get(KEY_PORT, KEY_DAH_PIN)){
				dash_press = 1;
			} else {
				if(dash_press){
					push(&morse_buf, 0x01);
					dash_press = 0;
					if (!dot_gen) { dash_gen = 1;}
				}

			}

			if(!gpio_get(KEY_PORT, KEY_DIT_PIN)){
				dot_press = 1;
			} else {
				if(dot_press){
					push(&morse_buf, 0x00);
					dot_press = 0;
					if (!dash_gen) { dot_gen = 1;}
				}
			}
			*/
			if(!gpio_get(KEY_PORT, KEY_DAH_PIN)){
				if (dash_press){
					if (!dot_gen) { dash_gen = 1;}
					//dash_press = 0;
				} else {
					dash_press = 1;
				}
			} else {
				dash_press = 0;
			}
			if(!gpio_get(KEY_PORT, KEY_DIT_PIN)){
				if (dot_press){
					if (!dash_gen) { dot_gen = 1;}
					//dot_press = 0;
				} else {
					dot_press = 1;
				}
			} else {
				dot_press = 0;
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
		main_screen_update_prescaler--;
		if (main_screen_update_prescaler == 0){
			main_screen_update_prescaler = SCREEN_REFRESH_PRESCALER;
			main_screen_update_flag = 1;
		}

	}
}

void setup_cw_len(uint8_t signal){
	//dot - 0.3 sec; dash - 0.9 sec
//	uint16_t dot = 200; //OK 
	uint16_t dot = 150; 
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
	if (timer_interrupt_source(TIM2, TIM_SR_UIF)) { //Mark (dash or dot) end
        	timer_clear_flag(TIM2, TIM_SR_UIF);
		timer_set_counter(TIM2, 0);
		/*uint8_t data = 0;
		if (pop(&morse_buf, &data)){ //Do we have dashes and dots in a buf?
			setup_cw_len(data); 
			tone_gen = 1;
			//si5351_enable_outputs(I2C2, SI5351_OUT_CLK_0);

			//-------------m62429_set_volume(snd, M62429_CH2, (uint8_t)radioCfg.toneVolLevel);
		} else {
			disable_tx();
			gpio_clear(GPIOA, GPIO0);
			gpio_clear(GPIOA, GPIO1);
		}*/
		if ((dash_gen) || (dot_gen)){
			enable_tx();
		} else {
			dot_gen = 0;
			dash_gen = 0;
			disable_tx();
		}

	}
	if (timer_interrupt_source(TIM2, TIM_SR_CC1IF)) { //Signal (rf and tone end)
        	timer_clear_flag(TIM2, TIM_SR_CC1IF);
		tone_gen = 0;
		si5351_disable_all_outputs(I2C2);
			dot_gen = 0;
			dash_gen = 0;
		//--------m62429_set_volume(snd, M62429_CH2, 0);	
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
		case 6: show_menu_backlight(); break;
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
		case FSM_BACKLIGHT_SELECT: roll_over_menu(rotary_pos); break;

		case FSM_RX_AUDIO_VOL_CHG: show_update_rx_audio_vol(rotary_pos); break;
		case FSM_TX_TONE_VOL_CHG: show_update_tx_tone_vol(rotary_pos); break;
		case FSM_TX_TONE_FREQ_CHG: show_update_tx_tone_freq(rotary_pos); break;
		case FSM_TX_OFFSET_CHG: show_update_tx_offset(rotary_pos); break;
		case FSM_STEP_CHG: show_update_step(rotary_pos); break;
		case FSM_CW_SPEED_CHG: show_update_cw_speed(rotary_pos); break;
		case FSM_BACKLIGHT_CHG: show_update_backlight(rotary_pos); break;
		default: break;
	}
}

void btn0_action(void){ //config-enter
 	__asm("nop");
}
void btn1_action(void){ //config-enter
	switch (ui_fsm_state){
		case FSM_MAIN_RX: show_menu_rx_audio_vol(); break;
		case FSM_RX_AUDIO_VOL_SELECT: radioCfg.currentMenuEntity = 0; show_edit_rx_audio_vol(); break;
		case FSM_TX_TONE_VOL_SELECT: radioCfg.currentMenuEntity = 1; show_edit_tx_tone_vol(); break;
		case FSM_TX_TONE_FREQ_SELECT: radioCfg.currentMenuEntity = 2; show_edit_tx_tone_freq(); break;
		case FSM_TX_OFFSET_SELECT: radioCfg.currentMenuEntity = 3; show_edit_tx_offset(); break;
		case FSM_STEP_SELECT: radioCfg.currentMenuEntity = 4; show_edit_step(); break;
		case FSM_CW_SPEED_SELECT: radioCfg.currentMenuEntity = 5; show_edit_cw_speed(); break;
		case FSM_BACKLIGHT_SELECT: radioCfg.currentMenuEntity = 6; show_edit_backlight(); break;
		default: break;
	}
}

void btn2_action(void){//exit-chg freq view
	switch (ui_fsm_state){
		case FSM_RX_AUDIO_VOL_CHG: save_cfg_on_flash(); show_menu_rx_audio_vol(); break;
		case FSM_TX_TONE_VOL_CHG: save_cfg_on_flash(); set_encoder(radioCfg.currentMenuEntity, MENU_MAX); show_menu_tx_tone_vol(); audio_switch2rx(); break;
		case FSM_TX_TONE_FREQ_CHG: save_cfg_on_flash(); set_encoder(radioCfg.currentMenuEntity, MENU_MAX); show_menu_tx_tone_freq(); audio_switch2rx();  break;
		case FSM_TX_OFFSET_CHG: save_cfg_on_flash(); set_encoder(radioCfg.currentMenuEntity, MENU_MAX); show_menu_tx_offset(); break;
		case FSM_STEP_CHG: save_cfg_on_flash(); set_encoder(radioCfg.currentMenuEntity, MENU_MAX); show_menu_step_select();  break;
		case FSM_CW_SPEED_CHG: save_cfg_on_flash(); set_encoder(radioCfg.currentMenuEntity, MENU_MAX); show_menu_cw_speed(); break;
		case FSM_BACKLIGHT_CHG: save_cfg_on_flash(); set_encoder(radioCfg.currentMenuEntity, MENU_MAX); show_menu_backlight(); break;
		default: show_main_screen();  break;
	}
}

/* Menu entities */

void show_menu_rx_audio_vol(void) { 
	ui_fsm_state = FSM_RX_AUDIO_VOL_SELECT; 
	set_encoder(0, MENU_MAX); 
        winstar_display(display, "*1* Recv", 1, 0); winstar_display(display, " snd lvl", 2, 0);
}
void show_menu_tx_tone_vol(void) { 
	ui_fsm_state = FSM_TX_TONE_VOL_SELECT;
        winstar_display(display, "*2* Tone", 1, 0); winstar_display(display, " snd lvl", 2, 0);
}
void show_menu_tx_tone_freq(void) { 
	ui_fsm_state = FSM_TX_TONE_FREQ_SELECT; 
        winstar_display(display, "*3* Tone", 1, 0); winstar_display(display, "  freq  ", 2, 0);
}
void show_menu_tx_offset(void) { 
	ui_fsm_state = FSM_TX_OFFSET_SELECT; 
        winstar_display(display, "*4* Trx ", 1, 0); winstar_display(display, " offset ", 2, 0);
}
void show_menu_step_select(void) { 
	ui_fsm_state = FSM_STEP_SELECT; 
        winstar_display(display, "*5* Step", 1, 0); winstar_display(display, "  size  ", 2, 0);
}
void show_menu_cw_speed(void) { 
	ui_fsm_state = FSM_CW_SPEED_SELECT; 
        winstar_display(display, "*6* CW", 1, 0); winstar_display(display, "TX speed", 2, 0);
}
void show_menu_backlight(void) { 
	ui_fsm_state = FSM_BACKLIGHT_SELECT; 
        winstar_display(display, "*7* Back", 1, 0); winstar_display(display, "light   ", 2, 0);
}

/* Menu entities end*/

/* Re-display settings*/
void redisplay_rx_audio_vol(void){ 
        winstar_display(display, "Recv lvl", 1, 0); 
        winstar_display(display, "  ", 2, 0);
	winstar_display_numeric(display, (uint16_t)(radioCfg.rxVolLevel), 2,  2, 2);
        winstar_display(display, "%  ", 2, 5);
}
void redisplay_tx_tone_vol(void){ 
        winstar_display(display, "Tone lvl", 1, 0); 
        winstar_display(display, "  ", 2, 0);
	winstar_display_numeric(display, (uint16_t)(radioCfg.toneVolLevel), 2,  2, 2);
        winstar_display(display, "%  ", 2, 5);
}
void redisplay_tx_tone_freq(void){ 
        winstar_display(display, "Tone frq", 1, 0); 
        winstar_display(display, "  ", 2, 0);
	winstar_display_numeric(display, (uint16_t)(radioCfg.toneFreq), 3,  2, 2);
        winstar_display(display, "  ", 2, 6);
}
void redisplay_tx_offset(void){ 
        winstar_display(display, "Tx offst", 1, 0); 
        winstar_display(display, "  ", 2, 0);
	winstar_display_numeric(display, (uint16_t)(radioCfg.txOffset), 3,  2, 2);
        winstar_display(display, "  ", 2, 6);
}
void redisplay_step(void){ 
        winstar_display(display, "Step chg", 1, 0); 
        winstar_display(display, " ", 2, 0);
	uint16_t v[2] = {1000, 100};
	winstar_display_numeric(display, v[(uint16_t)(radioCfg.stepSize)], 3,  2, 1);
        winstar_display(display, "Hz ", 2, 5);
}
void redisplay_cw_speed(void){ 
        winstar_display(display, "CW speed", 1, 0); 
        winstar_display(display, "   ", 2, 0);
	winstar_display_numeric(display, (uint16_t)(radioCfg.cwSpeed), 1, 2, 3);
        winstar_display(display, "   ", 2, 5);
}
void redisplay_backlight(void){ 
        winstar_display(display, "Bcklight", 1, 0); 
        winstar_display(display, "   ", 2, 0);
	winstar_display_numeric(display, (uint16_t)(display->backlight), 1, 2, 3);
        winstar_display(display, "   ", 2, 5);
}
/* Re-read and re-display settings end*/

/* Setting editor entities */

void show_edit_rx_audio_vol(void){ 
	ui_fsm_state = FSM_RX_AUDIO_VOL_CHG; 
	redisplay_rx_audio_vol();
	set_encoder(radioCfg.rxVolLevel, AUDIO_VOL_MAX); 
}
void show_edit_tx_tone_vol(void){ 
	ui_fsm_state = FSM_TX_TONE_VOL_CHG; 
	audio_switch2tx();
	redisplay_tx_tone_vol();
	set_encoder(radioCfg.toneVolLevel, TONE_VOL_MAX);
}
void show_edit_tx_tone_freq(void){ 
	ui_fsm_state = FSM_TX_TONE_FREQ_CHG; 
	audio_switch2tx();
	redisplay_tx_tone_freq();
	set_encoder(radioCfg.toneFreq, TONE_FREQ_MAX);
}
void show_edit_tx_offset(void){ 
	ui_fsm_state = FSM_TX_OFFSET_CHG; 
	redisplay_tx_offset();
	set_encoder(radioCfg.txOffset, OFFSET_MAX); 
}
void show_edit_step(void){ 
	ui_fsm_state = FSM_STEP_CHG; 
	redisplay_step();
	set_encoder(radioCfg.stepSize, STEP_MAX); 
}
void show_edit_cw_speed(void){ 
	ui_fsm_state = FSM_CW_SPEED_CHG; 
	redisplay_cw_speed();
	set_encoder(radioCfg.cwSpeed, CW_SPEED_MAX); 
}
void show_edit_backlight(void){ 
	ui_fsm_state = FSM_BACKLIGHT_CHG; 
	redisplay_backlight();
	set_encoder(display->backlight, BACKLIGHT_MAX); 
}

/* Setting editor entities end*/

/* Settings update actions */
void show_update_rx_audio_vol(uint32_t value){
	radioCfg.rxVolLevel = (uint16_t)value;
	m62429_set_volume(snd, M62429_CH1, (uint8_t)value);
	redisplay_rx_audio_vol();
}
void show_update_tx_tone_vol(uint32_t value){
	radioCfg.toneVolLevel = (uint16_t)value;
	m62429_set_volume(snd, M62429_CH2, (uint8_t)value);
	redisplay_tx_tone_vol();
}
void show_update_tx_tone_freq(uint32_t value){
	radioCfg.toneFreq = (uint16_t)value;
	redisplay_tx_tone_freq();
}
void show_update_tx_offset(uint32_t value){
	radioCfg.txOffset = (uint16_t)value;
	redisplay_tx_offset();
	rf_switch2rx();
}
void show_update_step(uint32_t value){
	radioCfg.stepSize = (uint16_t)value;
	redisplay_step();
}
void show_update_cw_speed(uint32_t value){
	radioCfg.cwSpeed = (uint16_t)value;
	redisplay_cw_speed();
}
void show_update_backlight(uint32_t value){
	display->backlight = (uint8_t)value;
	redisplay_backlight();
}
/* Settings update actions end */

