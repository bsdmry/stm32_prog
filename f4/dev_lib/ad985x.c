#include "ad985x.h"
void ad985x_init(uint32_t spi, uint32_t port_rst, uint16_t pin_rst){
	spi_setup(spi, SPI_CR1_BAUDRATE_FPCLK_DIV_16, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE, SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_8BIT, SPI_CR1_LSBFIRST);
	gpio_mode_setup(port_rst, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pin_rst);
	ad985x_reset(port_rst, pin_rst);
	spi_tx(spi, 0x0003);
}

void ad985x_reset(uint32_t port_rst, uint16_t pin_rst){
	gpio_set(port_rst, pin_rst);	
	for(uint8_t i=0;i<100;i++);
	gpio_clear(port_rst, pin_rst);	
	for(uint16_t i=0;i<1000;i++);
}

void ad985x_set_freq(uint32_t spi, uint32_t crystal_frequency, double frequency){
	uint8_t pf[5] = {0};
	int32_t f = frequency * 4294967295 / crystal_frequency;
	for (uint8_t i = 0; i < 4; i++){
		pf[i] = (uint8_t)(f & 0xFF);
		f>>=8;
	}
	spi_tx_array8(spi, pf, 5);
}

void ad9850_set_freq(uint32_t spi, double frequency){
	ad985x_set_freq(spi, 125000000, frequency);
}

void ad9851_set_freq(uint32_t spi, double frequency){
	ad985x_set_freq(spi, 30000000, frequency);
}
