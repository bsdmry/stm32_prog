#include "i2c.h"

void i2c_1_setup(void){
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_I2C1);

	/*i2c_reset(I2C1);
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO7);
	gpio_set_af(GPIOB, GPIO_AF4, GPIO6 | GPIO7);
	i2c_peripheral_disable(I2C1);
	i2c_set_speed(I2C1, i2c_speed_sm_100k, 8);
	i2c_set_standard_mode(I2C1);
	i2c_peripheral_enable(I2C1);*/
        gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO8);
        gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO8);
        gpio_set_af(GPIOB, GPIO_AF4, GPIO8);

        gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO9);
        gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO9);
        gpio_set_af(GPIOB, GPIO_AF4, GPIO9);

        i2c_peripheral_disable(I2C1); /* disable i2c during setup */
        i2c_reset(I2C1);
        i2c_set_own_7bit_slave_address(I2C1, 0x00);
	//i2c_set_standard_mode(I2C1);

        i2c_set_clock_frequency(I2C1, 48); //MY
        i2c_set_fast_mode(I2C1); //I2C1 -> CCR = 0x8028;
        i2c_set_ccr(I2C1, 40);   //I2C1 -> CCR = 0x8028;
        i2c_set_trise(I2C1, 15);
	
        i2c_peripheral_enable(I2C1); /* finally enable i2c */

}

void i2c_1_1_setup(void){
	rcc_periph_clock_enable(RCC_GPIOB);
    	rcc_periph_clock_enable(RCC_I2C1);
    	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO7);
    	gpio_set_af(GPIOB, GPIO_AF4, GPIO6 | GPIO7);
        gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO6 | GPIO7);
    	i2c_peripheral_disable(I2C1);
	i2c_reset(I2C1);
        //i2c_set_own_7bit_slave_address(I2C1, 0x00);
    	//configure ANFOFF DNF[3:0] in CR1
    	/* HSI is at 25Mhz */
    	//i2c_set_speed(I2C1, i2c_speed_sm_100k, 25);
    	i2c_set_speed(I2C1, i2c_speed_sm_100k, rcc_apb1_frequency/1000000);
	//i2c_set_standard_mode(I2C1);
    	i2c_peripheral_enable(I2C1);

}

void i2c_2_setup_411(void){
	rcc_periph_clock_enable(RCC_GPIOB);
    	rcc_periph_clock_enable(RCC_I2C2);
    	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO3 | GPIO10);
    	gpio_set_af(GPIOB, GPIO_AF9, GPIO3);
    	gpio_set_af(GPIOB, GPIO_AF4, GPIO10);
        gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO3 | GPIO10);
    	i2c_peripheral_disable(I2C2);
	i2c_reset(I2C2);
    	i2c_set_speed(I2C2, i2c_speed_sm_100k, rcc_apb1_frequency/1000000);
    	i2c_peripheral_enable(I2C2);
}
void i2c_2_setup_407(void){ //DO NOT WORK
	rcc_periph_clock_enable(RCC_GPIOB);
    	rcc_periph_clock_enable(RCC_I2C2);
    	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO10);
    	gpio_set_af(GPIOB, GPIO_AF4, GPIO11 | GPIO10);
        gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO11 | GPIO10);
    	i2c_peripheral_disable(I2C2);
	i2c_reset(I2C2);
    	i2c_set_speed(I2C2, i2c_speed_sm_100k, rcc_apb1_frequency/1000000);
    	i2c_peripheral_enable(I2C2);
}

void i2c_3_setup_407(void){
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOC);
    	rcc_periph_clock_enable(RCC_I2C3);
    	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO8); //SCL
    	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9); //SDA
    	gpio_set_af(GPIOA, GPIO_AF4, GPIO8);
    	gpio_set_af(GPIOC, GPIO_AF4, GPIO9);
        gpio_set_output_options(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO8);
        gpio_set_output_options(GPIOC, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO9);
    	i2c_peripheral_disable(I2C3);
	i2c_reset(I2C3);
    	i2c_set_speed(I2C3, i2c_speed_sm_100k, rcc_apb1_frequency/1000000);
    	i2c_peripheral_enable(I2C3);
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
