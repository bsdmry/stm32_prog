#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include "icom_ctl.h"
#include "usart.h"
#include "usb_cdc.h"
#include "i2c.h"
#include "lcd1602_i2c.h"
#include "cmd.h"

volatile uint8_t to_cdc_flag = 0;
volatile uint8_t screen_update_flag = 0;
char lcd_line1[17];
char lcd_line2[17];

Lcd1602_hbarline* volume_lvl;
Lcd1602_hbarline* signal_lvl;

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
	rcc_periph_clock_enable(RCC_TIM3);
}

static void tim3_setup(void)
{
        rcc_periph_reset_pulse(RST_TIM3);
        timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
        uint32_t prescaler = 4200;
        uint32_t period = 2000;
        timer_continuous_mode(TIM3);
        timer_set_prescaler(TIM3, prescaler -1 );
        timer_set_period(TIM3, period -1 );
        timer_set_counter(TIM3,0);
        /* Counter enable. */
        timer_enable_counter(TIM3);
        timer_enable_irq(TIM3, TIM_DIER_UIE);
        nvic_enable_irq(NVIC_TIM3_IRQ);
	nvic_set_priority(NVIC_TIM3_IRQ, 5);
}



int main(void)
{
	clock_setup();
	gpio_mode_setup(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8);
        gpio_clear(GPIOE, GPIO8);
	init_usb_cdc(1);
	tim3_setup();
        usart_setup(USART1, 9600, 8, USART_STOPBITS_1, USART_MODE_TX_RX, USART_PARITY_NONE, USART_FLOWCONTROL_NONE );
        usart_setup(USART2, 115200, 8, USART_STOPBITS_1, USART_MODE_TX, USART_PARITY_NONE, USART_FLOWCONTROL_NONE );
	nvic_enable_irq(NVIC_USART1_IRQ);
	usart_enable_rx_interrupt(USART1);
	nvic_set_priority(NVIC_USART1_IRQ, 2);

	i2c_1_1_setup();
	lcd1602_init(I2C1);
	lcd1602_set_custom_char(I2C1, 0, b1_chr);
	lcd1602_set_custom_char(I2C1, 1, b2_chr);
	lcd1602_set_custom_char(I2C1, 2, b3_chr);
	lcd1602_set_custom_char(I2C1, 3, b4_chr);
	lcd1602_set_custom_char(I2C1, 4, b5_chr);
	lcd1602_set_custom_char(I2C1, 5, ant_chr);
	lcd1602_set_custom_char(I2C1, 6, snd_chr);
	
	volume_lvl = lcd1602_init_horizontal_barline(255, 3, ' ', "\x00\x01\x02\x03\x04");
	signal_lvl = lcd1602_init_horizontal_barline(255, 7, ' ', "\x00\x01\x02\x03\x04");

	memset(lcd_line1, '-', 16);
	memset(lcd_line2, '-', 16);
	lcd1602_display(I2C1, lcd_line1, 1, 0);
	lcd1602_display(I2C1, lcd_line2, 2, 0);


	while(1){
		if (to_cdc_flag){
			to_cdc_flag = 0;
			cdcacm_data_tx(usbd_dev_g);
		}
		if (rx_ring_buffer.length > 0){
			 usart_enable_tx_interrupt(USART1);
		}
		if (screen_update_flag){
			lcd1602_set_horizontal_barline_value(volume_lvl, int_rcvr_params.volume);
			lcd1602_set_horizontal_barline_value(signal_lvl, int_rcvr_params.signalLevel);

	memset(lcd_line1, '-', 16);
	memset(lcd_line2, '-', 16);

			memcpy(&lcd_line1, "\x05", 1);
			memcpy(&lcd_line1[1], signal_lvl->bar_string, 7);
			memcpy(&lcd_line1[8], "\x06", 1);
			memcpy(&lcd_line1[9], volume_lvl->bar_string, 3);
			memcpy(&lcd_line1[12], char_freq_params.filter, 4);

			//memcpy(lcd_line1+5, int_rcvr_params.strAgc, 4);
			//memcpy(lcd_line1+9, int_rcvr_params.strNb, 3);
			//memcpy(lcd_line1+12, int_rcvr_params.strAtt, 4);

			memcpy(lcd_line2, char_freq_params.freq, 4);
			lcd_line2[4] = '.';
			memcpy(&lcd_line2[5], &char_freq_params.freq[4], 3);
			lcd_line2[8] = '.';
			memcpy(&lcd_line2[9], &char_freq_params.freq[7], 3);

			memcpy(&lcd_line2[13], char_freq_params.modulation, 3);

			lcd1602_display(I2C1, lcd_line1, 1, 0);
			lcd1602_display(I2C1, lcd_line2, 2, 0);
			screen_update_flag = 0;
		}
	}
}

void tim3_isr(void)
{
        if (timer_interrupt_source(TIM3, TIM_SR_UIF)) {
                screen_update_flag = 1;
                /* Clear compare interrupt flag. */
                timer_clear_flag(TIM3, TIM_SR_UIF);
        }
}

void usart1_isr(void){
	uint8_t data = 0;
	if (usart_get_flag(USART1, USART_SR_TXE)) {
                while(rb_u8_pop(&rx_ring_buffer, &data)){
			host_cmd_parser(data);
                        usart_send_blocking(USART1, data);
                }
                usart_disable_tx_interrupt(USART1);
                
        }
	if (usart_get_flag(USART1, USART_SR_RXNE)) {
                data = (uint8_t)usart_recv(USART1);
                rb_u8_push(&tx_ring_buffer, data);
		radio_reply_parser(data);
		to_cdc_flag = 1;
        }
}

void get_cdc_comm_config(uint32_t speed, uint8_t stop_bits, uint8_t parity, uint8_t data_bits){
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
}
