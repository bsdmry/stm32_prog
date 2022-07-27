#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/dac.h>
#include "dac1.h"

static void clock_setup(void)
{
	rcc_clock_setup_in_hse_8mhz_out_48mhz();
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOC);
        rcc_periph_clock_enable(RCC_DAC);
}

static void dac_setup(void){
    gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO4);    
    dac_disable(DAC1, DAC_CHANNEL1);
    //dac_trigger_disable(DAC1, DAC_CHANNEL1);
    dac_buffer_enable(DAC1, DAC_CHANNEL1);
    //dac_disable_waveform_generation(DAC1, DAC_CHANNEL1);
    //dac_set_trigger_source(DAC1, DAC_CR_TSEL1_SW);
    //dac_trigger_enable(DAC1, DAC_CHANNEL1);
    dac_enable(DAC1, DAC_CHANNEL1);
}

static void gpio_setup(void)
{
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO9);
	gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO9);
}

static void dl_loop(void){
	int i;
	for (i = 0; i < 3000000; i++) {
		__asm__("nop");
	}
}

int main(void)
{
	clock_setup();
	dl_loop();
	dac_setup();
	gpio_setup();
	dl_loop();
	dac_load_data_buffer_single(DAC1, 1000, DAC_ALIGN_RIGHT12, DAC_CHANNEL1);
	//dac_software_trigger(DAC1, DAC_CHANNEL1);
	while(1){
		gpio_toggle(GPIOC, GPIO9);
		dl_loop();
	}
}
