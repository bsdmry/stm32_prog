#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <stdlib.h>
#include "adc_test.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "cbuf.h"
#define ADC2EP_BUFFER_LENGTH 16
uint16_t adc_buffer0[16];
uint16_t adc_buffer1[16];
volatile uint8_t show_buf_flag = 0;
uint8_t adc_buffer_pointer = 1;
int16_t adc2ep[ADC2EP_BUFFER_LENGTH] = {0};

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_DMA2);
	rcc_periph_clock_enable(RCC_ADC1);
	rcc_periph_clock_enable(RCC_TIM2);
	rcc_periph_clock_enable(RCC_TIM3);
	rcc_periph_clock_enable(RCC_USART2);
}

static void adc_setup(void){
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);

	adc_power_off(ADC1);
	uint8_t channel_seq[16];
	channel_seq[0] = 0;
    	channel_seq[1] = 1;
	adc_set_regular_sequence(ADC1, 2, channel_seq);
	adc_set_single_conversion_mode(ADC1);
	adc_enable_scan_mode(ADC1);
	adc_eoc_after_each(ADC1);

	adc_set_right_aligned(ADC1);
	adc_set_resolution(ADC1, ADC_CR1_RES_12BIT);
	adc_set_sample_time(ADC1, ADC_CHANNEL0, ADC_SMPR_SMP_3CYC);
	adc_set_sample_time(ADC1, ADC_CHANNEL1, ADC_SMPR_SMP_3CYC);
	adc_enable_external_trigger_regular(ADC1, ADC_CR2_EXTSEL_TIM2_TRGO, ADC_CR2_EXTEN_RISING_EDGE);
	adc_disable_discontinuous_mode_regular(ADC1);
	adc_set_dma_continue(ADC1);
	adc_enable_dma(ADC1);
	adc_power_on(ADC1);

}
static void tim2_setup(void){
	rcc_periph_reset_pulse(RST_TIM2);
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

	uint32_t hz = 2;
	uint32_t prescaler = rcc_apb1_frequency / hz;
    	uint32_t period = 4;
	
    	while(prescaler>4800){
        	prescaler=prescaler/10;
        	period=period*10;
    	}
	
	timer_disable_preload(TIM2);
	timer_continuous_mode(TIM2);

	timer_set_prescaler(TIM2, prescaler -1 );
	timer_set_period(TIM2, period - 1);
	timer_set_counter(TIM2, 0); //Cleanup start value
	timer_set_master_mode(TIM2, TIM_CR2_MMS_UPDATE );
	timer_enable_counter(TIM2);
	//timer_enable_irq(TIM2, TIM_DIER_UIE);
	//nvic_enable_irq(NVIC_TIM2_IRQ);
}
static void tim3_setup(void){
	rcc_periph_reset_pulse(RST_TIM3);
	timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

	uint32_t hz = 2;
	uint32_t prescaler = rcc_apb1_frequency / hz;
    	uint32_t period = 2;
	
    	while(prescaler>4800){
        	prescaler=prescaler/10;
        	period=period*10;
    	}
	
	timer_disable_preload(TIM3);
	timer_continuous_mode(TIM3);

	timer_set_prescaler(TIM3, prescaler -1 );
	timer_set_period(TIM3, period - 1);
	timer_set_counter(TIM3, 0); //Cleanup start value
	timer_enable_counter(TIM3);
	timer_enable_irq(TIM3, TIM_DIER_UIE);
	nvic_enable_irq(NVIC_TIM3_IRQ);
}

static void dma_setup(void){
	nvic_enable_irq(NVIC_DMA2_STREAM0_IRQ);
	dma_stream_reset(DMA2, DMA_STREAM0);
	dma_set_priority(DMA2, DMA_STREAM0, DMA_SxCR_PL_LOW);
	dma_set_memory_size(DMA2, DMA_STREAM0, DMA_SxCR_MSIZE_16BIT);
	dma_set_peripheral_size(DMA2, DMA_STREAM0, DMA_SxCR_PSIZE_16BIT);
	dma_enable_memory_increment_mode(DMA2, DMA_STREAM0);
	dma_enable_circular_mode(DMA2, DMA_STREAM0);
	dma_set_transfer_mode(DMA2, DMA_STREAM0, DMA_SxCR_DIR_PERIPHERAL_TO_MEM);
	dma_set_peripheral_address(DMA2, DMA_STREAM0, (uint32_t) &ADC_DR(ADC1));
	dma_enable_double_buffer_mode(DMA2, DMA_STREAM0);
	dma_set_memory_address(DMA2, DMA_STREAM0, (uint32_t) adc_buffer0);
	dma_set_memory_address_1(DMA2, DMA_STREAM0, (uint32_t) adc_buffer1);
	dma_set_number_of_data(DMA2, DMA_STREAM0, 16);
	dma_enable_transfer_complete_interrupt(DMA2, DMA_STREAM0);
	dma_channel_select(DMA2, DMA_STREAM0, DMA_SxCR_CHSEL_0);
	dma_enable_stream(DMA2, DMA_STREAM0);	
}

static void usart_setup(void)
{
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);
	gpio_set_af(GPIOA, GPIO_AF7, GPIO2);
	/* Setup USART2 parameters. */
	usart_set_baudrate(USART2, 115200);
	usart_set_databits(USART2, 8);
	usart_set_stopbits(USART2, USART_STOPBITS_1);
	usart_set_mode(USART2, USART_MODE_TX);
	usart_set_parity(USART2, USART_PARITY_NONE);
	usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

	/* Finally enable the USART. */
	usart_enable(USART2);
}

void dma2_stream0_isr(void)
{
	if (dma_get_interrupt_flag(DMA2, DMA_STREAM0, DMA_TCIF)) {
                uint16_t *  b;
                if (adc_buffer_pointer){
                        b = adc_buffer1;
                } else {
                        b = adc_buffer0;
                }
                int32_t temp = 0;
                for (uint16_t i = 0; i < 16; i++){
                        //temp = (int32_t)(b[i] - 32768);
                        temp = (int32_t)(b[i] - 16384);
                        buf_push(&ring_buffer, (int16_t)temp);
                }

                adc_buffer_pointer = adc_buffer_pointer ^ 0x1;
                dma_clear_interrupt_flags(DMA2, DMA_STREAM0, DMA_TCIF);
		dma_clear_interrupt_flags(DMA2, DMA_STREAM0, DMA_TCIF);
		/* Toggle PC1 just to keep aware of activity and frequency. */
		//gpio_toggle(GPIOC, GPIO13);
	}
}

void tim3_isr(void)
{
	if (timer_get_flag(TIM3, TIM_SR_UIF)) {
		show_buf_flag = 1;
		/* Clear compare interrupt flag. */
		timer_clear_flag(TIM3, TIM_SR_UIF);
	}	
}

void usart_send_string(uint8_t *pStr, uint16_t len){
	for (uint16_t i = 0; i < len; i++){
		usart_send_blocking(USART2, (uint16_t)pStr[i]);
	}
}

void show_buf(uint8_t half_buf_len){
        uint16_t mem_size = sizeof(char) * 31 * half_buf_len;
        char* buf = malloc(mem_size);
        for (uint8_t i = 0; i < half_buf_len; i++){
                sprintf(buf + 31*i, "0x%04"PRIx16" 0x%04"PRIx16"\t0x%04"PRIx16" 0x%04"PRIx16"\r\n", adc_buffer0[i*2], adc_buffer0[i*2+1], adc_buffer1[i*2], adc_buffer1[i*2+1]);
                //sprintf(buf + 15*i, "0x%04"PRIx16" 0x%04"PRIx16"\r\n", adc_buffer0[i*2], adc_buffer0[i*2+1]);

        }
        uint16_t mem_size1 = sizeof(char) * 16 * ADC2EP_BUFFER_LENGTH/2;
	char* buf1 = malloc(mem_size1);
        for (uint8_t i = 0; i < half_buf_len; i++){
                sprintf(buf1 + 16*i, "%+06"PRId16"  %+06"PRId16"\r\n", adc2ep[i*2], adc2ep[i*2+1]);
        }
	char blength[55];
	sprintf(blength, "Buf flag: %03"PRIo8" Head: %03"PRIo8" Tail: %03"PRIo8" Buffer length: %03"PRIo8"\r\n", adc_buffer_pointer, ring_buffer.head, ring_buffer.tail, ring_buffer.length);
	usart_send_string(">>>>>>>\r\n", 9);
	usart_send_string(buf, mem_size);
	usart_send_string(buf1, mem_size1);
	usart_send_string(blength, 54);
	usart_send_string("<<<<<<<\r\n", 9);
        free(buf);
        free(buf1);
}

int main(void)
{
	clock_setup();
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
        gpio_set(GPIOC, GPIO13);
	usart_setup();
	dma_setup();
	adc_setup();
	tim3_setup();
	tim2_setup();
	while(1){
		if (show_buf_flag){
			show_buf(8);	
			show_buf_flag = 0;
		}
		if (ring_buffer.length > 64){
                	gpio_clear(GPIOC, GPIO13);
                        for (uint16_t i = 0; i < ADC2EP_BUFFER_LENGTH; i++){
                                buf_pop(&ring_buffer, &adc2ep[i]);
                        }
		}
	}
}
