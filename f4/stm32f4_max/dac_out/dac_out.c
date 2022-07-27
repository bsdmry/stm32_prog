
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/dac.h>

uint8_t waveform[256];

/* Set STM32 to 168 MHz. */
static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOE);
        rcc_periph_clock_enable(RCC_DAC);
}


static void dac_setup(void){
    gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO4);    
    dac_disable(DAC1, DAC_CHANNEL1);
    dac_trigger_disable(DAC1, DAC_CHANNEL1);
    dac_buffer_enable(DAC1, DAC_CHANNEL1);
    dac_disable_waveform_generation(DAC1, DAC_CHANNEL1);
    dac_set_trigger_source(DAC1, DAC_CR_TSEL1_SW);
    dac_trigger_enable(DAC1, DAC_CHANNEL1);
    dac_enable(DAC1, DAC_CHANNEL1);

    /*dac_trigger_disable(DAC1, DAC_CHANNEL1);
    dac_set_waveform_generation(DAC1, DAC_CHANNEL1, DAC_WAVE_TRIANGLE);
    dac_set_waveform_characteristics(DAC1, DAC_CHANNEL1, 12);
    dac_buffer_enable(DAC1, DAC_CHANNEL1);
    dac_trigger_enable(DAC1, DAC_CHANNEL1);
    dac_set_trigger_source(DAC1, DAC_CR_TSEL1_SW);
    dac_enable(DAC1, DAC_CHANNEL1);*/

}

static void gpio_setup(void)
{
	gpio_mode_setup(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8);
	gpio_set_output_options(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO8);
}

static void dl_loop(void){
	int i;
	for (i = 0; i < 3000000; i++) { /* Wait a bit. */
		__asm__("nop");
	}
}

int main(void)
{
	clock_setup();
	gpio_setup();
	dac_setup();
	dl_loop();
	while(1){
		dac_load_data_buffer_single(DAC1, 4000, DAC_ALIGN_RIGHT12, DAC_CHANNEL1);
		dac_software_trigger(DAC1, DAC_CHANNEL1);
		gpio_toggle(GPIOE, GPIO8);
		dl_loop();
	}
	return 0;
}
