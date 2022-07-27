#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/gpio.h>
#include "max7219_ctl.h"

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
  //gpio_clear(GPIOA, GPIO6);
  gpio_set(MAXPORT, MAX_CS);

  /* Reset SPI, SPI_CR1 register cleared, SPI is disabled */
  spi_disable(MAX_SPI_DEV);

  /* Set up SPI in Master mode with:
   * Clock baud rate: 1/64 of peripheral clock frequency
   * Clock polarity: Idle High
   * Clock phase: Data valid on 2nd clock pulse
   * Data frame format: 8-bit
   * Frame format: MSB First
   */
  spi_init_master(MAX_SPI_DEV, SPI_CR1_BAUDRATE_FPCLK_DIV_128, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE, SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);

  /*spi_set_master_mode(MAX_SPI_DEV);
  spi_set_full_duplex_mode(MAX_SPI_DEV);
  spi_set_baudrate_prescaler(MAX_SPI_DEV, SPI_CR1_BR_FPCLK_DIV_128);
  spi_set_clock_polarity_0(MAX_SPI_DEV);
  spi_set_clock_phase_0(MAX_SPI_DEV);
  spi_set_dff_8bit(MAX_SPI_DEV);
  spi_send_msb_first(MAX_SPI_DEV); 
  spi_disable_crc(MAX_SPI_DEV); 
  spi_enable_software_slave_management(MAX_SPI_DEV);
  //spi_disable_ss_output(MAX_SPI_DEV);
  spi_set_nss_high(MAX_SPI_DEV);*/

  /* Enable SPI1 periph. */
  spi_enable(MAX_SPI_DEV);
}

static void spi_tx(int32_t spi, uint8_t data){
        while (!(SPI_SR(spi) & SPI_SR_TXE));
 	//gpio_clear(MAXPORT, MAX_CS);
        SPI_DR(spi) = data;
        while (!(SPI_SR(spi) & SPI_SR_TXE));
	while (!(SPI_SR(spi) & SPI_SR_RXNE));
	(void)SPI_DR(spi);
	while(!(SPI_SR(spi) & SPI_SR_TXE));
	while(SPI_SR(spi) & SPI_SR_BSY);
  	//gpio_set(MAXPORT, MAX_CS);
}

static void send_max71219_cmd(uint8_t cmd, uint8_t data){
	uint16_t c = 0;
	c = c & cmd;
	c = c << 8;
	c = c & data;
 	gpio_clear(MAXPORT, MAX_CS);
	for (int i = 0; i < 100; ++i) {
            __asm__("nop");
        }
	spi_tx(MAX_SPI_DEV, cmd);
	spi_tx(MAX_SPI_DEV, data);
  	gpio_set(MAXPORT, MAX_CS);
}

int main(void)
{
	clock_setup();

	spi_setup();

	send_max71219_cmd(0x09, 0x00);
	send_max71219_cmd(0x0B, 0x07);
	send_max71219_cmd(0x0A, 0x03);
	send_max71219_cmd(0x0C, 0x01);

	send_max71219_cmd(0x01, 0x01);
	send_max71219_cmd(0x02, 0x03);
	send_max71219_cmd(0x03, 0x07);
	send_max71219_cmd(0x04, 0x0F);
	send_max71219_cmd(0x05, 0x1F);
	send_max71219_cmd(0x06, 0x3F);
	send_max71219_cmd(0x07, 0x7F);
	send_max71219_cmd(0x08, 0xFF);

}

