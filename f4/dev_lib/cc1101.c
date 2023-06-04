#include "spi.h"
#include "dwt.h"
#include "cc1101.h"

void cc1101_set_gd_pin(void){
	rcc_periph_clock_enable(RCC_GPIOA);
	gpio_mode_setup(GD_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GDO0_PIN | GDO2_PIN);	
}

void cc1101_write_strobe(uint8_t strobe)
{
	spi_tx8(SPI2, strobe);
}

uint8_t cc1101_read_reg(uint32_t spi, uint8_t reg){
	
	spi_set_cs(spi, 0);
	spi_xfer(spi, (uint16_t)(READ(reg)));
	uint8_t res = (uint8_t )spi_xfer(spi, 0);
	spi_set_cs(spi, 1);
	return res;
}

void cc1101_read_burst_reg(uint32_t spi, uint8_t reg, uint8_t* data, uint8_t data_len){
	spi_set_cs(spi, 0);
	spi_xfer(spi, (uint16_t)(READ_BURST(reg)));
	for (uint8_t i = 0; i < data_len; i++){
		data[i] = (uint8_t )spi_xfer(spi, 0);
	}
	spi_set_cs(spi, 1);
}

void cc1101_write_reg(uint32_t spi, uint8_t reg, uint8_t val){
	spi_set_cs(spi, 0);
	spi_xfer(spi, (uint16_t)(WRITE(reg)));
	spi_xfer(spi, (uint16_t)(val));
	spi_set_cs(spi, 1);
}

void cc1101_write_burst_reg(uint32_t spi, uint8_t reg, uint8_t* data, uint8_t data_len){
	spi_set_cs(spi, 0);
	spi_xfer(spi, (uint16_t)(WRITE_BURST(reg)));
	for (uint8_t i = 0; i < data_len; i++){
		spi_xfer(spi, (uint16_t)(data[i]));
	}
	spi_set_cs(spi, 1);
}

uint8_t cc1101_check(void){
	uint8_t result = 0;
	uint8_t version;
	for(uint8_t i=0; i<10; i++){
		version = cc1101_read_reg(SPI2, VERSION);
		if ((version!=0x14) | (version != 0x04)) {
			result = 1;
		}
	}
	return result;
}


/*Init DWT subsystem first!!!*/
void cc1101_init(void)
{
	spi_setup(SPI2, SPI_CR1_BAUDRATE_FPCLK_DIV_64, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE, SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);
	cc1101_set_gd_pin();
	cc1101_reset();
}

void cc1101_reset(void){
	rcc_periph_clock_enable(SPI2_A_PORT_RCC);

	/* Reinit CS pin, and set it HIGH */
	gpio_mode_setup(SPI2_A_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI2_A_CS);
	spi_set_cs(SPI2, 1);

	/*Reset SPI pin setup, and set pins as GPIO*/
        rcc_periph_clock_disable(RCC_SPI2); //Disable SPI if it was set before
	gpio_mode_setup(SPI2_A_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI2_A_MOSI | SPI2_A_SCK);	
	gpio_mode_setup(SPI2_A_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI2_A_MISO);
	/* bring SPI lines to a defined state. Reasons are outlined in CC1101 datasheet - section 11.3*/
	gpio_clear(SPI2_A_PORT, SPI2_A_MOSI); //MOSI to up
	gpio_set(SPI2_A_PORT, SPI2_A_SCK); //SCK to down
		
	spi_set_cs(SPI2, 0);
	//dwt_delay_us(10);  // Toggle CS
	spi_set_cs(SPI2, 1);
	dwt_delay_us(45);
	spi_set_cs(SPI2, 0);

	/*Wait until chip starts oscillator*/
	while(gpio_get(SPI2_A_PORT, SPI2_A_MISO)){}

	/*Re-init SPI subsystem for the RESET command*/
	spi_setup(SPI2, SPI_CR1_BAUDRATE_FPCLK_DIV_64, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE, SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);
	cc1101_write_strobe(CC1101_SRES);

	/*Reset SPI pin setup again*/
        rcc_periph_clock_disable(RCC_SPI2);
	gpio_mode_setup(SPI2_A_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI2_A_MOSI | SPI2_A_SCK);	
	gpio_mode_setup(SPI2_A_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI2_A_MISO);

	/*Wait until chip starts oscillator*/
	while(gpio_get(SPI2_A_PORT, SPI2_A_MISO)){}

	/*Restore SPI configuration*/
	spi_set_cs(SPI2, 1);
	spi_setup(SPI2, SPI_CR1_BAUDRATE_FPCLK_DIV_64, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE, SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);


}

void cc1101_set_cfg(uint8_t freq){
	cc1101_write_reg(SPI2, CC1101_FSCTRL1,  0x08);
    	cc1101_write_reg(SPI2, CC1101_FSCTRL0,  0x00);
	
    	switch(freq)
    	{
      		case F_868:
      			cc1101_write_reg(SPI2, CC1101_FREQ2,    F2_868);
      			cc1101_write_reg(SPI2, CC1101_FREQ1,    F1_868);
      			cc1101_write_reg(SPI2, CC1101_FREQ0,    F0_868);
        		break;
      		case F_915:
        		cc1101_write_reg(SPI2, CC1101_FREQ2,    F2_915);
        		cc1101_write_reg(SPI2, CC1101_FREQ1,    F1_915);
        		cc1101_write_reg(SPI2, CC1101_FREQ0,    F0_915);
        		break;
	  	case F_433:
        		cc1101_write_reg(SPI2, CC1101_FREQ2,    F2_433);
        		cc1101_write_reg(SPI2, CC1101_FREQ1,    F1_433);
        		cc1101_write_reg(SPI2, CC1101_FREQ0,    F0_433);
        		break;
	  	default: // F must be set
	  		break;
	}
	
    	cc1101_write_reg(SPI2, CC1101_MDMCFG4,  0x5B);
    	cc1101_write_reg(SPI2, CC1101_MDMCFG3,  0xF8);
    	cc1101_write_reg(SPI2, CC1101_MDMCFG2,  0x03);
    	cc1101_write_reg(SPI2, CC1101_MDMCFG1,  0x22);
    	cc1101_write_reg(SPI2, CC1101_MDMCFG0,  0xF8);
    	cc1101_write_reg(SPI2, CC1101_CHANNR,   0x00);
    	cc1101_write_reg(SPI2, CC1101_DEVIATN,  0x47);
    	cc1101_write_reg(SPI2, CC1101_FREND1,   0xB6);
    	cc1101_write_reg(SPI2, CC1101_FREND0,   0x10);
    	cc1101_write_reg(SPI2, CC1101_MCSM0 ,   0x18);
    	cc1101_write_reg(SPI2, CC1101_FOCCFG,   0x1D);
    	cc1101_write_reg(SPI2, CC1101_BSCFG,    0x1C);
    	cc1101_write_reg(SPI2, CC1101_AGCCTRL2, 0xC7);
	cc1101_write_reg(SPI2, CC1101_AGCCTRL1, 0x00);
    	cc1101_write_reg(SPI2, CC1101_AGCCTRL0, 0xB2);
    	cc1101_write_reg(SPI2, CC1101_FSCAL3,   0xEA);
	cc1101_write_reg(SPI2, CC1101_FSCAL2,   0x2A);
	cc1101_write_reg(SPI2, CC1101_FSCAL1,   0x00);
    	cc1101_write_reg(SPI2, CC1101_FSCAL0,   0x11);
    	cc1101_write_reg(SPI2, CC1101_FSTEST,   0x59);
    	cc1101_write_reg(SPI2, CC1101_TEST2,    0x81);
    	cc1101_write_reg(SPI2, CC1101_TEST1,    0x35);
    	cc1101_write_reg(SPI2, CC1101_TEST0,    0x09);
    	cc1101_write_reg(SPI2, CC1101_IOCFG2,   0x0B); 	//serial clock.synchronous to the data in synchronous serial mode
    	cc1101_write_reg(SPI2, CC1101_IOCFG0,   0x06);  	//asserts when sync word has been sent/received, and de-asserts at the end of the packet 
    	cc1101_write_reg(SPI2, CC1101_PKTCTRL1, 0x04);		//two status bytes will be appended to the payload of the packet,including RSSI LQI and CRC OK
											//No address check
    	cc1101_write_reg(SPI2, CC1101_PKTCTRL0, 0x05);		//whitening off;CRC Enable£»variable length packets, packet length configured by the first byte after sync word
    	cc1101_write_reg(SPI2, CC1101_ADDR,     0x00);		//address used for packet filtration.
    	cc1101_write_reg(SPI2, CC1101_PKTLEN,   0x3D); 	//61 bytes max length

}

void cc1101_send_data(uint8_t* data, uint8_t data_len){
	cc1101_write_reg(SPI2, CC1101_TXFIFO, data_len);
	cc1101_write_burst_reg(SPI2, CC1101_TXFIFO, data, data_len);
	cc1101_write_strobe(CC1101_STX);
	while (!gpio_get(GD_PORT, GDO0_PIN));
	while (gpio_get(GD_PORT, GDO0_PIN));
	cc1101_write_strobe(CC1101_SFTX);
}

uint8_t cc1101_check_rx_flag(void){
	if (gpio_get(GD_PORT, GDO0_PIN)){
		while (gpio_get(GD_PORT, GDO0_PIN));
		return 1;
	} else {
		return 0;
	}
}

uint8_t cc1101_read_data(uint8_t* data ){
	uint8_t size;
	uint8_t status[2];

	if (cc1101_read_reg(SPI2, RXBYTES) & BYTES_IN_RXFIFO_MASK){
		size = cc1101_read_reg(SPI2, CC1101_RXFIFO);
		cc1101_read_burst_reg(SPI2, CC1101_RXFIFO, data, size);
		cc1101_read_burst_reg(SPI2, CC1101_RXFIFO, status, 2);
		cc1101_write_strobe(CC1101_SFRX);
		return size;
	}
	else
	{
		cc1101_write_strobe(CC1101_SFRX);
		return 0;
	}
}
