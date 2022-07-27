#include <libopencm3/stm32/rcc.h>
#include "i2c.h"
#include "i2c_pcf8574a.h"
#define PCF_ADDR 0x3F

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_I2C1);
}


int main(void)
{
	clock_setup();
	i2c_1_setup();
	uint8_t msg[5] = {0x01, 0xAA, 0x11, 0xDA, 0x14} ;
	//i2c_transfer7(I2C1, PCF_ADDR, msg, 5, 0, 0);
	i2c_write_reg(I2C1, PCF_ADDR, 0xAF, msg, 5);
	while(1){

	}
}
