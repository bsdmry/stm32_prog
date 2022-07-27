#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "dac_test.h"

#define DACPORT         GPIOA
#define DAC_RCC_SPI     RCC_SPI1
#define DAC_RCC_GPIO    RCC_GPIOA
#define DAC_LDAC        GPIO3
#define DAC_CS          GPIO4
#define DAC_CLK         GPIO5
#define DAC_MISO        GPIO6
#define DAC_MOSI        GPIO7
#define DAC_SPI_DEV     SPI1

uint8_t sine_point = 0;

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);
	rcc_periph_clock_enable(DAC_RCC_GPIO);
        rcc_periph_clock_enable(DAC_RCC_SPI);
	rcc_periph_clock_enable(RCC_TIM2);
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

static void tim_setup(void)
{

	rcc_periph_reset_pulse(RST_TIM2);
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

	/*
	 * Please take note that the clock source for STM32 timers
	 * might not be the raw APB1/APB2 clocks.  In various conditions they
	 * are doubled.  See the Reference Manual for full details!
	 * 
	 */

	/*uint32_t hz = 2048000;
	uint32_t prescaler = rcc_apb1_frequency / hz;
    	uint32_t period = 2;*/
	
	uint32_t prescaler = 375;
    	uint32_t period = 2;

	
    	while(prescaler>4800){
        	prescaler=prescaler/10;
        	period=period*10;
    	}
	
	timer_disable_preload(TIM2);
	timer_continuous_mode(TIM2);

	timer_set_prescaler(TIM2, prescaler -1 );
	timer_set_period(TIM2, period - 1);
	timer_set_counter(TIM2, 0); //Cleanup start value

	timer_enable_counter(TIM2);
	timer_enable_irq(TIM2, TIM_DIER_UIE);
	nvic_enable_irq(NVIC_TIM2_IRQ);
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

void dac_send(uint16_t a, uint16_t b){
	spi_tx(DAC_SPI_DEV, ((a & 0x0FFF) ^ 0x7000) ); //b15 - 0 (DAC_A), bit14 - 1 (buffered), bit13 - 1(no gain), bit12 - 1(active mode)
	spi_tx(DAC_SPI_DEV, ((b & 0x0FFF) ^ 0xF000) ); //b15 - 1 (DAC_B), bit14 - 1 (buffered), bit13 - 1(no gain), bit12 - 1(active mode)
	//spi_tx(DAC_SPI_DEV, 0x7FA0);
	//spi_tx(DAC_SPI_DEV, 0xF7D0);
	gpio_clear(DACPORT, DAC_LDAC); 
	for (int i = 0; i < 3; ++i) {
            __asm__("nop");
        }
  	gpio_set(DACPORT, DAC_LDAC); //Load Latch
}

int main(void)
{
	clock_setup();
	spi_setup();
	tim_setup();
	dac_send(0x0FA0, 0x07D0); //4000, 2000
	while (1) {
	}
}

void tim2_isr(void)
{
	if (timer_get_flag(TIM2, TIM_SR_UIF)) {
		//dac_send(sine256[sine_point], sine256[(uint8_t)(sine_point + 63)]);
		dac_send(sine16[sine_point], sine16[sine_point]);
		sine_point++;
		if (sine_point > 15){
			sine_point = 0;
		}
		/* Clear compare interrupt flag. */
		timer_clear_flag(TIM2, TIM_SR_UIF);
	}	
}
