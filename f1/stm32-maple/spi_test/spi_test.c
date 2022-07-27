#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/gpio.h>
#include "spi_test.h"

#define MAXPORT 	GPIOB
#define MAX_RCC_SPI 	RCC_SPI2
#define MAX_RCC_GPIO 	RCC_GPIOB
#define MAX_CS		GPIO11
#define MAX_CLK 	GPIO13
#define MAX_MISO	GPIO14
#define MAX_MOSI	GPIO15
#define MAX_SPI_DEV	SPI2

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

	rcc_periph_clock_enable(MAX_RCC_GPIO);
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(MAX_RCC_SPI);
}

static void spi_setup(void){
	
  gpio_set_mode(MAXPORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, MAX_CS);
  gpio_set_mode(MAXPORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, MAX_CLK | MAX_MOSI );
  gpio_set_mode(MAXPORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, MAX_MISO);
  gpio_set(MAXPORT, MAX_CS);

  spi_disable(MAX_SPI_DEV);

  spi_init_master(MAX_SPI_DEV, SPI_CR1_BAUDRATE_FPCLK_DIV_128, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE, SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_16BIT, SPI_CR1_MSBFIRST);

  /*spi_set_master_mode(MAX_SPI_DEV);
  spi_set_full_duplex_mode(MAX_SPI_DEV);
  spi_set_baudrate_prescaler(MAX_SPI_DEV, SPI_CR1_BR_FPCLK_DIV_32);
  spi_set_clock_polarity_0(MAX_SPI_DEV);
  spi_set_clock_phase_0(MAX_SPI_DEV);
  spi_set_dff_16bit(MAX_SPI_DEV);
  spi_send_msb_first(MAX_SPI_DEV); 
  spi_disable_crc(MAX_SPI_DEV); 
  spi_enable_software_slave_management(MAX_SPI_DEV);
  spi_set_nss_high(MAX_SPI_DEV);
  */
  spi_enable(MAX_SPI_DEV);
}

static void spi_tx(int32_t spi, uint16_t data){
        while (!(SPI_SR(spi) & SPI_SR_TXE));
 	gpio_clear(MAXPORT, MAX_CS);
        SPI_DR(spi) = data;
        while (!(SPI_SR(spi) & SPI_SR_TXE));
	while (!(SPI_SR(spi) & SPI_SR_RXNE));
	(void)SPI_DR(spi);
	while(!(SPI_SR(spi) & SPI_SR_TXE));
	while(SPI_SR(spi) & SPI_SR_BSY);
  	gpio_set(MAXPORT, MAX_CS);
}

int main(void)
{
	clock_setup();
	spi_setup();

	for (int i = 0; i < 1650000; ++i) {
            __asm__("nop");
        }

	spi_tx(MAX_SPI_DEV, 0xDEAD);
}

