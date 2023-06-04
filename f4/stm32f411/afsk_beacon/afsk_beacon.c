#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <string.h>
#include "ax25.h"
#include "hdlc.h"
#include "afsk_beacon.h"


#define DACPORT         GPIOA
#define DAC_RCC_SPI     RCC_SPI1
#define DAC_RCC_GPIO    RCC_GPIOA
#define DAC_LDAC        GPIO3
#define DAC_CS          GPIO4
#define DAC_CLK         GPIO5
#define DAC_MISO        GPIO6
#define DAC_MOSI        GPIO7
#define DAC_SPI_DEV     SPI1
#define SINETABLE_SIZE	40

volatile uint8_t tx_char = 0;
volatile uint8_t byte_index = 0;

volatile uint16_t byte_pos = 0;
volatile uint8_t bit_pos = 0;
volatile uint8_t state = 1;
uint8_t sine_point = 0;
volatile hdlc_frame* hdlcDataFrame;

enum BitPeriod {
	MARK = 1000,
	SPACE = 545,

};

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(DAC_RCC_GPIO);
        rcc_periph_clock_enable(DAC_RCC_SPI);
	rcc_periph_clock_enable(RCC_TIM3);
	rcc_periph_clock_enable(RCC_TIM4);
}

static void gpio_setup(void){

	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13 | GPIO15);
	gpio_set(GPIOC, GPIO13);
	gpio_clear(GPIOC, GPIO15);
}

static void tim3_setup(void){
	rcc_periph_reset_pulse(RST_TIM3);
	timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_enable_preload(TIM3);
	timer_continuous_mode(TIM3);
	uint32_t hz = 60000; //bitrate 1200, set counter speed hundred times faster
	uint32_t prescaler = 48000000 / hz - 1;
	timer_set_prescaler(TIM3, prescaler );
	timer_set_period(TIM3, 100 -1 ); //one bit [100-1]
	
	timer_set_counter(TIM3, 0);
	timer_enable_counter(TIM3);
	
	timer_enable_irq(TIM3, TIM_DIER_UIE);
	nvic_enable_irq(NVIC_TIM3_IRQ);
}

static void tim4_setup(void)
{
	rcc_periph_reset_pulse(RST_TIM4);
	timer_set_mode(TIM4, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	
	uint32_t prescaler = 2; //rcc_apb1_frequency = 48000000 / 2 = 24 Mhz
	uint32_t period = MARK;

	
    	while(prescaler>4800){
        	prescaler=prescaler/10;
        	period=period*10;
    	}
	
	timer_disable_preload(TIM4);
	timer_continuous_mode(TIM4);

	timer_set_prescaler(TIM4, prescaler -1 );
	timer_set_period(TIM4, period - 1);
	timer_set_counter(TIM4, 0); //Cleanup start value

	timer_enable_counter(TIM4);
	timer_enable_irq(TIM4, TIM_DIER_UIE);
	nvic_enable_irq(NVIC_TIM4_IRQ);
}

static void spi_setup(void){
	gpio_mode_setup(DACPORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, DAC_CS | DAC_LDAC);

  	gpio_mode_setup(DACPORT, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, DAC_CLK | DAC_MOSI | DAC_MISO );
  	gpio_set_af(DACPORT, GPIO_AF5, DAC_MOSI | DAC_MISO | DAC_CLK);
	gpio_set_output_options(DACPORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, DAC_MOSI | DAC_CLK);
  	gpio_set(DACPORT, DAC_CS);
  	gpio_set(DACPORT, DAC_LDAC);

	spi_disable(DAC_SPI_DEV);
	spi_init_master(DAC_SPI_DEV, SPI_CR1_BAUDRATE_FPCLK_DIV_2, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE, SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_16BIT, SPI_CR1_MSBFIRST);
	spi_enable(DAC_SPI_DEV);	
}

void tim3_isr(void)
{
	if (timer_interrupt_source(TIM3, TIM_SR_UIF)) {
		//gpio_toggle(GPIOC, GPIO15);
		//
		uint8_t flag = 0;
		if (bit_pos < 8 ){  flag |= (1<<0); } else { flag &= ~(1<<0); }
                if (byte_pos < hdlcDataFrame->frameSize ) { flag |= (1<<1); } else { flag &= ~(1<<1); }
		switch (flag){
			case 0: //Out of byte size & out of string size. End
				bit_pos = 0;
				byte_pos = 0;
				hdlcDataFrame->frameSize = 0;
				free(hdlcDataFrame->frameData);
				timer_disable_counter(TIM4);
				timer_disable_counter(TIM3);
				timer_set_counter(TIM4, 0);
				timer_set_counter(TIM3, 0);
				break;
			case 1: //In byte size, out of string size. End			
				bit_pos = 0;
				byte_pos = 0;
				hdlcDataFrame->frameSize = 0;
				free(hdlcDataFrame->frameData);
				timer_disable_counter(TIM4);
				timer_disable_counter(TIM3);
				timer_set_counter(TIM4, 0);
				timer_set_counter(TIM3, 0);
				break;
			case 2: //Out of byte size, still in string size.
				break;
			case 3: //In byte size, in string size.
				if (!(hdlcDataFrame->frameData[byte_pos] & 0x01)){
					state = state ^ 0x01;
					if (state) {
						timer_set_period(TIM4, MARK - 1 );
					//	timer_generate_event(TIM4, TIM_EGR_UG);
					} else {
						timer_set_period(TIM4, SPACE - 1 );
					//	timer_generate_event(TIM4, TIM_EGR_UG);
					}
				}
				hdlcDataFrame->frameData[byte_pos] >>= 1;
				bit_pos++;
				if (bit_pos >= 8 ) { bit_pos = 0;  byte_pos++; }
				break;
			default:
				break;
		}
		/* Clear compare interrupt flag. */
		timer_clear_flag(TIM3, TIM_SR_UIF);
	}	
}

void tim4_isr(void)
{
	if (timer_get_flag(TIM4, TIM_SR_UIF)) {
		gpio_toggle(GPIOC, GPIO15);
		dac_send(sine32[sine_point]);
		sine_point++;
		if (sine_point > SINETABLE_SIZE -1){
			sine_point = 0;
		}
		/* Clear compare interrupt flag. */
		timer_clear_flag(TIM4, TIM_SR_UIF);
	}	
}

void spi_tx(int32_t spi, uint16_t data){
        while (!(SPI_SR(spi) & SPI_SR_TXE));
        gpio_clear(DACPORT, DAC_CS);
        SPI_DR(spi) = data;
        while (!(SPI_SR(spi) & SPI_SR_TXE));
        while (!(SPI_SR(spi) & SPI_SR_RXNE));
        (void)SPI_DR(spi);
        while(!(SPI_SR(spi) & SPI_SR_TXE));
        while(SPI_SR(spi) & SPI_SR_BSY);
        gpio_set(DACPORT, DAC_CS);
}

void dac_send(uint16_t a){
	spi_tx(DAC_SPI_DEV, ((a & 0x0FFF) ^ 0x7000) ); //b15 - 0 (DAC_A), bit14 - 1 (buffered), bit13 - 1(no gain), bit12 - 1(active mode)
	gpio_clear(DACPORT, DAC_LDAC); 
	for (int i = 0; i < 3; ++i) {
            __asm__("nop");
        }
  	gpio_set(DACPORT, DAC_LDAC); //Load Latch
}

void tx_data(void){
	//msg_len = strlen(strData);
	//msg_len = 5;
	sine_point = 0;
	timer_set_period(TIM4, MARK - 1);
//	timer_generate_event(TIM4, TIM_EGR_UG);
	timer_enable_counter(TIM4);
	//timer_enable_counter(TIM3);
}



int main(void)
{
	clock_setup();
	gpio_setup();
	spi_setup();
	tim4_setup();
	tim3_setup();

	/*aprs_msg aprsMsg;
	setReciever(&aprsMsg, "NJ7P", 0);
   	setSender(&aprsMsg, "N7LEM", 1, 1);
   	setCtrlField(&aprsMsg);
   	setPidField(&aprsMsg);
   	setPayload(&aprsMsg, "1 2 3");*/
	tx_data();
	while(1){
		for (int i = 0; i < 20000000; ++i) {
            		__asm__("nop");
        	}
		gpio_toggle(GPIOC, GPIO13);
   	//	hdlcDataFrame = hdlcMakeFrame(&aprsMsg);
		//tx_data();
	};
}





