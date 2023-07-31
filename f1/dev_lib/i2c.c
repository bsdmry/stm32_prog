#include "i2c.h"

void i2c_1_setup(void){
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_I2C1);
	
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ , GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN , GPIO8 | GPIO9);
	gpio_primary_remap(AFIO_MAPR_SWJ_CFG_FULL_SWJ, AFIO_MAPR_I2C1_REMAP);

	i2c_peripheral_disable(I2C1);
	i2c_reset(I2C1);
	i2c_set_own_7bit_slave_address(I2C1, 0x00);
	i2c_set_speed(I2C1, i2c_speed_sm_100k, rcc_apb1_frequency/1000000);
	i2c_peripheral_enable(I2C1);
}

void i2c_1_1_setup(void){
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_I2C1);
	
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ , GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN , GPIO6 | GPIO7);
	//gpio_primary_remap(AFIO_MAPR_SWJ_CFG_FULL_SWJ, AFIO_MAPR_I2C1_REMAP);

	i2c_peripheral_disable(I2C1);
	i2c_reset(I2C1);
	i2c_set_own_7bit_slave_address(I2C1, 0x00);
	i2c_set_speed(I2C1, i2c_speed_sm_100k, rcc_apb1_frequency/1000000);
	i2c_peripheral_enable(I2C1);
}

void i2c_2_setup(void){
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_AFIO);
	rcc_periph_clock_enable(RCC_I2C2);
	
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ , GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN , GPIO10 | GPIO11);

	i2c_peripheral_disable(I2C2);
	i2c_reset(I2C2);
	i2c_set_own_7bit_slave_address(I2C2, 0x00);
	i2c_set_speed(I2C2, i2c_speed_sm_100k, rcc_apb1_frequency/1000000);
	i2c_peripheral_enable(I2C2);
}

void i2c_write_reg(uint32_t i2c, uint8_t dev_addr, uint8_t reg, uint8_t *data, uint8_t data_len)
{
	while(I2C_SR2(i2c) & I2C_SR2_BUSY){}
	//i2c_enable_ack(i2c);
	i2c_send_start(i2c);
	while(
	!((I2C_SR1(i2c) & I2C_SR1_SB) &&
	(I2C_SR2(i2c) & I2C_SR2_MSL) &&
	(I2C_SR2(i2c) & I2C_SR2_BUSY))
	){}
	i2c_send_7bit_address(i2c, dev_addr, I2C_WRITE);
	while(!(I2C_SR1(i2c) & I2C_SR1_ADDR)){}
	(void)I2C_SR2(i2c);
	i2c_send_data(i2c, reg);
 	while (!(I2C_SR1(i2c) & (I2C_SR1_BTF)));
	for(uint8_t i = 0; i < data_len; i++){
		i2c_send_data(i2c, data[i]);
 		while (!(I2C_SR1(i2c) & (I2C_SR1_BTF)));
	}
	i2c_send_stop(i2c);
	//https://github.com/blippy/rpi/blob/master/stm32f411re/libopencm3/11-ledmat-i2c/main.c	
	//https://kunalsalvi63.medium.com/bare-metal-i2c-driver-for-stm32f411cex-3cc600fdcc05
}
void i2c_write_reg_single(uint32_t i2c, uint8_t dev_addr, uint8_t reg, uint8_t data){
	i2c_write_reg(i2c, dev_addr, reg, &data, 1);
}
