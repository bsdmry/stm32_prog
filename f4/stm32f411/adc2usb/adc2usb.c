#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/audio.h>
#include <libopencm3/usb/dwc/otg_fs.h>
#include <libopencm3/usb/dwc/otg_common.h>

#include "adc2usb.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "if_audiocontrol.h"
#include "if_audiostream.h"
#include "cbuf.h"

#define VENDOR_ID           0x1209  /* pid.code */
#define PRODUCT_ID          0x70b1  /* Assigned to Tomu project */
#define DEVICE_VER          0x0101  /* Program version */

#define ADC_BUFFER_LENGTH 16
#define ADC2EP_BUFFER_LENGTH 32

usbd_device *g_usbd_dev = 0;
static char usb_serial_number[25];
static const char * usb_strings[] = {
    "libopencm3.org",
    "AUDIO demo",
    usb_serial_number
};
uint8_t usbd_control_buffer[128];
uint16_t adc_buffer0[ADC_BUFFER_LENGTH];
uint16_t adc_buffer1[ADC_BUFFER_LENGTH];
int16_t adc2ep[ADC2EP_BUFFER_LENGTH] = {5000};
int16_t waveform_data[32] = {
	0,  32767,  12539,  30272,  23169,  23169,  30272,  12539,
        32767, 0,  30272, -12539,  23169, -23169,  12539, -30272,
        0, -32767, -12539, -30272, -23169, -23169, -30272, -12539,
       -32767, 0, -30272,  12539, -23169,  23169, -12539,  30272 
};


static const struct usb_device_descriptor dev = {
    .bLength = USB_DT_DEVICE_SIZE,
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB = 0x0200,    /* was 0x0110 in Table B-1 example descriptor */
    .bDeviceClass = 0,   /* device defined at interface level */
    .bDeviceSubClass = 1,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = VENDOR_ID,
    .idProduct = PRODUCT_ID,
    .bcdDevice = DEVICE_VER,
    .iManufacturer = 1,  /* index to string desc */
    .iProduct = 2,       /* index to string desc */
    .iSerialNumber = 3,  /* index to string desc */
    .bNumConfigurations = 1,
};

uint8_t streaming_iface_cur_altsetting = 0;

static const struct usb_interface ifaces[] = {{
    .num_altsetting = 1,
    .altsetting = audio_control_iface,
}, {
    .num_altsetting = 2,
    .cur_altsetting = &streaming_iface_cur_altsetting,
    .altsetting = audio_streaming_iface,
} };

static const struct usb_config_descriptor config = {
    .bLength = USB_DT_CONFIGURATION_SIZE,
    .bDescriptorType = USB_DT_CONFIGURATION,
    .wTotalLength = 0, /* can be anything, it is updated automatically
                          when the usb code prepares the descriptor */
    .bNumInterfaces = 2, /* control and streaming */
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = 0x80, /* bus powered */
    .bMaxPower = 0x32,
    .interface = ifaces,
};

uint8_t adc_buffer_pointer = 1;

static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_DMA2);
	rcc_periph_clock_enable(RCC_ADC1);
	rcc_periph_clock_enable(RCC_TIM3);
    	rcc_periph_clock_enable(RCC_OTGFS);
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

	adc_set_left_aligned(ADC1);
	adc_set_resolution(ADC1, ADC_CR1_RES_12BIT);
	adc_set_sample_time(ADC1, ADC_CHANNEL0, ADC_SMPR_SMP_3CYC);
	adc_set_sample_time(ADC1, ADC_CHANNEL1, ADC_SMPR_SMP_3CYC);
	adc_enable_external_trigger_regular(ADC1, ADC_CR2_EXTSEL_TIM3_TRGO, ADC_CR2_EXTEN_RISING_EDGE);
	adc_disable_discontinuous_mode_regular(ADC1);
	adc_set_dma_continue(ADC1);
	adc_enable_dma(ADC1);
	adc_power_on(ADC1);
}

static void tim3_setup(void){
	rcc_periph_reset_pulse(RST_TIM3);
	timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

	uint32_t hz = 16000;
	uint32_t prescaler = rcc_apb1_frequency / hz;
    	uint32_t period = 4;
	
    	while(prescaler>4800){
        	prescaler=prescaler/10;
        	period=period*10;
    	}
	
	timer_disable_preload(TIM3);
	timer_continuous_mode(TIM3);

	timer_set_prescaler(TIM3, prescaler -1 );
	timer_set_period(TIM3, period - 1);
	timer_set_counter(TIM3, 0); //Cleanup start value
	timer_set_master_mode(TIM3, TIM_CR2_MMS_UPDATE );
	timer_enable_counter(TIM3);
}

static void usb_setup(void){
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12);
	gpio_set_af(GPIOA, GPIO_AF10, GPIO11 | GPIO12);
	g_usbd_dev = usbd_init(&otgfs_usb_driver, &dev, &config,
                   usb_strings, 3, usbd_control_buffer,
                   sizeof(usbd_control_buffer));
	OTG_FS_GCCFG |= OTG_GCCFG_NOVBUSSENS | OTG_GCCFG_PWRDWN;
	OTG_FS_GCCFG &= ~(OTG_GCCFG_VBUSBSEN | OTG_GCCFG_VBUSASEN);
	usbd_register_set_config_callback(g_usbd_dev, usbaudio_set_config);
	nvic_enable_irq(NVIC_OTG_FS_IRQ);	
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
	dma_set_number_of_data(DMA2, DMA_STREAM0, ADC_BUFFER_LENGTH );
	dma_enable_transfer_complete_interrupt(DMA2, DMA_STREAM0);
	dma_channel_select(DMA2, DMA_STREAM0, DMA_SxCR_CHSEL_0);
	dma_enable_stream(DMA2, DMA_STREAM0);	
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
		for (uint16_t i = 0; i < ADC_BUFFER_LENGTH; i++){
			//temp = (int32_t)(b[i] - 32768);
			temp = (int32_t)(b[i] - 16384);
			buf_push_s16(&ring_buffer, (int16_t)temp);
		}
		
		adc_buffer_pointer = ~adc_buffer_pointer;
		//adc_buffer_pointer = adc_buffer_pointer ^ 0x1;
		dma_clear_interrupt_flags(DMA2, DMA_STREAM0, DMA_TCIF);
	}
}

void otg_fs_isr(void)
{
	usbd_poll(g_usbd_dev);
}

#define USB_REBASE(x) MMIO32((x) + (USB_OTG_FS_BASE))
#define USB_DIEPCTLX_SD1PID     (1 << 29) /* Odd frames */
#define USB_DIEPCTLX_SD0PID     (1 << 28) /* Even frames */
void toggle_isochronous_frame(uint8_t ep)
{
    static int toggle = 0;
    if (toggle++ % 2 == 0) {
        USB_REBASE(OTG_DIEPCTL(ep)) |= USB_DIEPCTLX_SD0PID;
    } else {
        USB_REBASE(OTG_DIEPCTL(ep)) |= USB_DIEPCTLX_SD1PID;
    }
}

void endpoint_callback(usbd_device *usbd_dev, uint8_t ep)
{
    toggle_isochronous_frame(ep);
    usbd_ep_write_packet(usbd_dev, 0x82, adc2ep, ADC2EP_BUFFER_LENGTH*2);
    //usbd_ep_write_packet(usbd_dev, 0x82, waveform_data, ADC2EP_BUFFER_LENGTH*2);
}

void usbaudio_set_config(usbd_device *usbd_dev, uint16_t wValue)
{
    (void)wValue;
	//16 kHz per ch * 2 ch * 2 bytes (int16_t) / 1000 (1ms) = 64 bytes BYTES!!! Not int16 chunks!
    usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_ISOCHRONOUS, ADC2EP_BUFFER_LENGTH*2, endpoint_callback);
    usbd_ep_write_packet(usbd_dev, 0x82, adc2ep, ADC2EP_BUFFER_LENGTH*2);
    //usbd_ep_write_packet(usbd_dev, 0x82, waveform_data, ADC2EP_BUFFER_LENGTH*2);
}

int main(void)
{
	clock_setup();
	dma_setup();
	adc_setup();
	tim3_setup();
	usb_setup();
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
	gpio_set(GPIOC, GPIO13);
	while(1){
		if (ring_buffer.length >= 128){
			gpio_clear(GPIOC, GPIO13);
			for (uint16_t i = 0; i < ADC2EP_BUFFER_LENGTH; i++){
				buf_pop_s16(&ring_buffer, &adc2ep[i]);
			}
		}
	}
}
