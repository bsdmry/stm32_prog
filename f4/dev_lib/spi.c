#include "spi.h"
/** @brief Configure the SPI as Master.
@param[in] spi Unsigned int32. SPI peripheral identifier
@param[in] dff Unsigned int32. Data frame format  SPI_CR1_DFF_16BIT |  SPI_CR1_DFF_8BIT
*/
void spi_setup(
		uint32_t  	spi,
		uint32_t  	br,
		uint32_t  	cpol,
		uint32_t  	cpha,
		uint32_t  	dff,
		uint32_t  	lsbfirst ){
	if (spi == SPI1){
		rcc_periph_clock_enable(SPI1_A_PORT_RCC);
        	rcc_periph_clock_enable(RCC_SPI1);
  		gpio_mode_setup(SPI1_A_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, SPI1_A_SCK | SPI1_A_MOSI | SPI1_A_MISO );
  		gpio_set_af(SPI1_A_PORT, GPIO_AF5, SPI1_A_MOSI | SPI1_A_MISO | SPI1_A_SCK);
		gpio_set_output_options(SPI1_A_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI1_A_MOSI | SPI1_A_SCK);
		gpio_mode_setup(SPI1_A_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI1_A_CS);
  		gpio_set(SPI1_A_PORT, SPI1_A_CS);

		spi_disable(SPI1);
		spi_init_master(SPI1, br, cpol, cpha, dff, lsbfirst);
		spi_enable(SPI1);
	} else if (spi == SPI2) {
		rcc_periph_clock_enable(SPI2_A_PORT_RCC);
        	rcc_periph_clock_enable(RCC_SPI2);
  		gpio_mode_setup(SPI2_A_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, SPI2_A_SCK | SPI2_A_MISO | SPI2_A_MOSI );
  		gpio_set_af(SPI2_A_PORT, GPIO_AF5, SPI2_A_SCK | SPI2_A_MISO | SPI2_A_MOSI);
		gpio_set_output_options(SPI2_A_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI2_A_SCK | SPI2_A_MOSI);
		gpio_mode_setup(SPI2_A_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI2_A_CS);
  		gpio_set(SPI2_A_PORT, SPI2_A_CS);

		spi_disable(SPI2);
		spi_init_master(SPI2, br, cpol, cpha, dff, lsbfirst);
		spi_enable(SPI2);
		
	} else {
		rcc_periph_clock_enable(SPI1_A_PORT_RCC);
        	rcc_periph_clock_enable(RCC_SPI1);
  		gpio_mode_setup(SPI1_A_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, SPI1_A_SCK | SPI1_A_MOSI | SPI1_A_MISO );
  		gpio_set_af(SPI1_A_PORT, GPIO_AF5, SPI1_A_MOSI | SPI1_A_MISO | SPI1_A_SCK);
		gpio_set_output_options(SPI1_A_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI1_A_MOSI | SPI1_A_SCK);
		gpio_mode_setup(SPI1_A_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI1_A_CS);
  		gpio_set(SPI1_A_PORT, SPI1_A_CS);

		spi_disable(SPI1);
		spi_init_master(SPI1, br, cpol, cpha, dff, lsbfirst);
		spi_enable(SPI1);
	}
	
}

void spi_set_cs(int32_t spi, uint8_t state){
	uint32_t CS_PORT;
	uint16_t CS_PIN;
	if (spi == SPI1){
		CS_PORT = SPI1_A_PORT;
		CS_PIN = SPI1_A_CS;
	} else if (spi == SPI2) {
		CS_PORT = SPI2_A_PORT;
		CS_PIN = SPI2_A_CS;
	} else {
		CS_PORT = SPI1_A_PORT;
		CS_PIN = SPI1_A_CS;
	}
	if (state){
        	gpio_set(CS_PORT, CS_PIN);
	} else {
        	gpio_clear(CS_PORT, CS_PIN);
	}
}

void spi_tx_array16(int32_t spi, uint16_t* data, uint16_t len){
        while (!(SPI_SR(spi) & SPI_SR_TXE));
	spi_set_cs(spi, 0);
	for (uint16_t i = 0; i < len; i++){
        	SPI_DR(spi) = data[i];
        	while (!(SPI_SR(spi) & SPI_SR_TXE));
        	while (!(SPI_SR(spi) & SPI_SR_RXNE));
	}
        (void)SPI_DR(spi);
        while(!(SPI_SR(spi) & SPI_SR_TXE));
        while(SPI_SR(spi) & SPI_SR_BSY);
	spi_set_cs(spi, 1);
}

void spi_tx_array16_nocs(int32_t spi, uint16_t* data, uint16_t len){
	for (uint16_t i = 0; i < len; i++){
        	SPI_DR(spi) = data[i];
        	while (!(SPI_SR(spi) & SPI_SR_TXE));
        	while (!(SPI_SR(spi) & SPI_SR_RXNE));
	}
        (void)SPI_DR(spi);
        while(!(SPI_SR(spi) & SPI_SR_TXE));
        while(SPI_SR(spi) & SPI_SR_BSY);
}

void spi_tx_array8(int32_t spi, uint8_t* data, uint16_t len){
	uint16_t* a = (uint16_t*)malloc(len);
	for (uint16_t i = 0; i < len; i++){
		a[i] = (uint16_t)data[i];
	}
	spi_tx_array16(spi, a, len);
	free(a);
}
void spi_tx(int32_t spi, uint16_t data){
	spi_tx_array16(spi, &data, 1);
}

void spi_tx_nocs(int32_t spi, uint16_t data){
	spi_tx_array16_nocs(spi, &data, 1);
}

void spi_tx8(int32_t spi, uint8_t data){
	spi_tx_array8(spi, &data, 1);
}

uint8_t spi_rx_reg8(int32_t spi, uint8_t reg){
	uint8_t result;
        while (!(SPI_SR(spi) & SPI_SR_TXE));
	spi_set_cs(spi, 0);
        SPI_DR(spi) = (uint16_t)reg;
       	while (!(SPI_SR(spi) & SPI_SR_TXE));
        while (!(SPI_SR(spi) & SPI_SR_RXNE));
	result =  (uint8_t)SPI_DR(spi);
        (void)SPI_DR(spi);
        while(!(SPI_SR(spi) & SPI_SR_TXE));
        while(SPI_SR(spi) & SPI_SR_BSY);
	spi_set_cs(spi, 1);
	return result;
}
