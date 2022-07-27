#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/dac.h>
#include <libopencm3/stm32/dma.h>
#include "dac_example.h"
#include "fifo.h"

#define BUFFER_SIZE 128
#define FIFO_SIZE 256
static uint16_t DAC_DMA_buffer[BUFFER_SIZE];
static uint32_t ADC_DMA_buffer[BUFFER_SIZE];
FIFO_buffer ADC_fifo;
FIFO_buffer DAC_fifo;

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
	/* Port A and C are on AHB1 */
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_GPIOA);
}

static void gpio_setup(void)
{
	/* Set the digital test output on PC1 */
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);
	gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO1);

	/* Set PA4 for DAC channel 1 to analogue, ignoring drive mode. 
	 * Set PA0 for ADC channel 0
	 * */
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0 | GPIO4);
}

static void timer_setup(void)
{
	/* Enable TIM2 clock. */
	//TIM2 feed: APB1 (now 42Mhz)
	rcc_periph_clock_enable(RCC_TIM2);
	rcc_periph_reset_pulse(RST_TIM2);
	/* Timer global mode: - No divider, Alignment edge, Direction up */
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_continuous_mode(TIM2);
	timer_set_period(TIM2, 874); //Count till 875
	timer_disable_oc_output(TIM2, TIM_OC2 | TIM_OC3 | TIM_OC4);
	timer_enable_oc_output(TIM2, TIM_OC1);
	timer_disable_oc_clear(TIM2, TIM_OC1);
	timer_disable_oc_preload(TIM2, TIM_OC1);
	timer_set_oc_slow_mode(TIM2, TIM_OC1);
	timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_TOGGLE);
	timer_set_oc_value(TIM2, TIM_OC1, 874); //Call compare event on 874
	timer_disable_preload(TIM2);
	/* Set the timer trigger output (for the DAC) to the channel 1 output
	   compare */
	timer_set_master_mode(TIM2, TIM_CR2_MMS_COMPARE_OC1REF);
	timer_enable_counter(TIM2);
}

static void dac_setup(void)
{
	/* Enable the DAC clock on APB1 */
	rcc_periph_clock_enable(RCC_DAC);
	/* Setup the DAC channel 1, with timer 2 as trigger source.
	 * Assume the DAC has woken up by the time the first transfer occurs */
	dac_trigger_enable(DAC1, DAC_CHANNEL1);
	dac_set_trigger_source(DAC1, DAC_CR_TSEL1_T2); //TIM2_TRGO
	dac_dma_enable(DAC1, DAC_CHANNEL1);
	dac_enable(DAC1, DAC_CHANNEL1);
}

static void dma_dac_setup(void)
{
	/* DAC channel 1 uses DMA controller 1 Stream 5 Channel 7. */
	/* Enable DMA1 clock and IRQ */
	rcc_periph_clock_enable(RCC_DMA1);
	nvic_enable_irq(NVIC_DMA1_STREAM5_IRQ);
	dma_stream_reset(DMA1, DMA_STREAM5);
	dma_set_priority(DMA1, DMA_STREAM5, DMA_SxCR_PL_LOW);
	dma_set_memory_size(DMA1, DMA_STREAM5, DMA_SxCR_MSIZE_16BIT);
	dma_set_peripheral_size(DMA1, DMA_STREAM5, DMA_SxCR_PSIZE_16BIT);
	dma_enable_memory_increment_mode(DMA1, DMA_STREAM5);
	dma_enable_circular_mode(DMA1, DMA_STREAM5);
	dma_set_transfer_mode(DMA1, DMA_STREAM5, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);

	dma_set_peripheral_address(DMA1, DMA_STREAM5, (uint32_t) &DAC_DHR12R1(DAC1));
	dma_set_memory_address(DMA1, DMA_STREAM5, (uint32_t) DAC_DMA_buffer);
	
	dma_set_number_of_data(DMA1, DMA_STREAM5, BUFFER_SIZE);
	dma_enable_half_transfer_interrupt(DMA1, DMA_STREAM5);
	dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM5);
	dma_channel_select(DMA1, DMA_STREAM5, DMA_SxCR_CHSEL_7);
	dma_enable_stream(DMA1, DMA_STREAM5);
}

static void adc_setup(void){

	rcc_periph_clock_enable(RCC_ADC1);
	rcc_periph_clock_enable(RCC_ADC2);
	adc_power_off(ADC1);
	adc_power_off(ADC2);
	adc_set_clk_prescale(ADC_CCR_ADCPRE_BY8);
	adc_disable_scan_mode(ADC1);
	adc_disable_scan_mode(ADC2);
	adc_set_single_conversion_mode(ADC1);
	adc_set_single_conversion_mode(ADC2);
	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_84CYC);
	adc_set_sample_time_on_all_channels(ADC2, ADC_SMPR_SMP_84CYC);
	adc_set_right_aligned(ADC1);
	adc_set_right_aligned(ADC2);
	adc_set_resolution(ADC1, ADC_CR1_RES_12BIT);
	adc_set_resolution(ADC2, ADC_CR1_RES_12BIT);
	uint8_t sequence1[] = { ADC_CHANNEL0 };
	uint8_t sequence2[] = { ADC_CHANNEL1 };
	adc_set_regular_sequence(ADC1, 1, sequence1);
	adc_set_regular_sequence(ADC2, 1, sequence2);
	/* Enable dual simultaneous conversion and DMA mode 2 */
	adc_set_multi_mode(ADC_CCR_MULTI_DUAL_REGULAR_SIMUL | ADC_CCR_DMA_MODE_2);
	adc_enable_external_trigger_regular(ADC1, ADC_CR2_EXTSEL_TIM2_TRGO, ADC_CR2_EXTEN_RISING_EDGE);    // TIM2_TRGO event
	adc_set_dma_continue(ADC1);
	adc_enable_dma(ADC1);
	adc_power_on(ADC1);
	adc_power_on(ADC2);

}

static void dma_adc_setup(void){
	/* ADC1 uses DMA controller 2 Stream 0/4 Channel 0. */
	rcc_periph_clock_enable(RCC_DMA2);
	nvic_enable_irq(NVIC_DMA2_STREAM0_IRQ);
	dma_stream_reset(DMA2, DMA_STREAM0);
	dma_set_priority(DMA2, DMA_STREAM0, DMA_SxCR_PL_MEDIUM);
	/* Configure memory/peripheral */
	/* Normally use ADC1_DR as 16-bit right justified data register */
	/* Note that in dual mode we use ADC_CDR, which in DMA mode 2 contains 32 bits of data */
	dma_set_memory_size(DMA2, DMA_STREAM0, DMA_SxCR_MSIZE_32BIT);
	dma_set_peripheral_size(DMA2, DMA_STREAM0, DMA_SxCR_PSIZE_32BIT);
	dma_set_peripheral_address(DMA2, DMA_STREAM0, (uint32_t) &ADC_CDR);
	dma_set_memory_address(DMA2, DMA_STREAM0, (uint32_t) ADC_DMA_buffer);
	dma_set_number_of_data(DMA2, DMA_STREAM0, BUFFER_SIZE);

	dma_enable_memory_increment_mode(DMA2, DMA_STREAM0);
	dma_enable_circular_mode(DMA2, DMA_STREAM0);
	dma_set_transfer_mode(DMA2, DMA_STREAM0, DMA_SxCR_DIR_PERIPHERAL_TO_MEM);
	dma_enable_transfer_complete_interrupt(DMA2, DMA_STREAM0);
	dma_enable_half_transfer_interrupt(DMA2, DMA_STREAM0);
	dma_channel_select(DMA2, DMA_STREAM0, DMA_SxCR_CHSEL_0);
	dma_enable_stream(DMA2, DMA_STREAM0);
}

//Mem to DAC
void dma1_stream5_isr(void) {
	/* The ISR simply provides a test output for a CRO trigger */
	if (dma_get_interrupt_flag(DMA1, DMA_STREAM5, DMA_TCIF)) {
		/* Transfer complete */
		dma_clear_interrupt_flags(DMA1, DMA_STREAM5, DMA_TCIF);
		/* Fill the second half of the DMA buffer from the FIFO */
		fill_dac_dma_buffer(BUFFER_SIZE / 2, BUFFER_SIZE);
	}
	if (dma_get_interrupt_flag(DMA1, DMA_STREAM5, DMA_HTIF)) {
		/* Half of transfer complete */
		dma_clear_interrupt_flags(DMA1, DMA_STREAM5, DMA_HTIF);
		/* Fill the first half of the DMA buffer from the FIFO */
		fill_dac_dma_buffer(0, BUFFER_SIZE / 2);
	}
}
//ADC to mem
void dma2_stream0_isr(void) {
	/* The ISR simply provides a test output for a CRO trigger */
	if (dma_get_interrupt_flag(DMA2, DMA_STREAM0, DMA_TCIF)) {
		/* Transfer complete */
		dma_clear_interrupt_flags(DMA2, DMA_STREAM0, DMA_TCIF);
		/* Append the second half of the DMA buffer to the FIFO */
		fill_adc_dma_buffer(BUFFER_SIZE / 2, BUFFER_SIZE);
	}
	if (dma_get_interrupt_flag(DMA2, DMA_STREAM0, DMA_HTIF)) {
		/* Half of transfer complete */
		dma_clear_interrupt_flags(DMA2, DMA_STREAM0, DMA_HTIF);
		/* Append the first half of the DMA buffer to the FIFO */
		fill_adc_dma_buffer(0, BUFFER_SIZE / 2);
	}
}
//
static void fill_adc_dma_buffer(int idx_from, int idx_to) {
	for (int idx = idx_from; idx < idx_to; idx++) {
		uint32_t value = ADC_DMA_buffer[idx];
		if (fifo_put(ADC_fifo, &value) != FifoActionStatus.FIFO_OVERRUN) {
		//if (!adc_ch1_2_queue.put(value)) {
			// FIFO overrun
		}
	}
}
static void fill_dac_dma_buffer(int idx_from, int idx_to) {
	for (int idx = idx_from; idx < idx_to; idx++) {
		uint32_t value = 0;
		if (fifo_get(DAC_fifo, &value) != FifoActionStatus.FIFO_UNDERRUN) {
		//if (!dac_ch1_queue.get(value)) {
			// FIFO underrun
		}
		DAC_DMA_buffer[idx] = value;
	}
}



int main(void)
{
	*ADC_fifo = fifo_init(FIFO_SIZE);
	*DAC_fifo = fifo_init(FIFO_SIZE);
	clock_setup();
	gpio_setup();
	timer_setup();
	dma_setup();
	dac_setup();

	while (1);

	return 0;
}
