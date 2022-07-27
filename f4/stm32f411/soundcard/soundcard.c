/*
 * Basic USB audio streaming example.
 *
 * Implements a stereo 8KHz microphone.
 *
 * Copyright (C) 2018 Seb Holzapfel <schnommus@gmail.com>
 *
 * Licensed under the Apache Licence, Version 2.0 (the "Licence");
 * you may not use this file except in compliance with the Licence.
 *  You may obtain a copy of the Licence at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the Licence is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the Licence for the specific language governing permissions and
 * limitations under the Licence.
 */

/*#include <libopencm3/cm3/common.h>
#include <libopencm3/cm3/vector.h>
#include <libopencm3/cm3/scb.h>*/
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/audio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/dwc/otg_fs.h>
#include <libopencm3/usb/dwc/otg_common.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "if_audiocontrol.h"
#include "if_audiostream.h"

//#define LED_GREEN_PORT      GPIOE
//#define LED_GREEN_PIN       GPIO8
#define LED_RED_PORT        GPIOC
#define LED_RED_PIN         GPIO13

#define VENDOR_ID           0x1209  /* pid.code */
#define PRODUCT_ID          0x70b1  /* Assigned to Tomu project */
#define DEVICE_VER          0x0101  /* Program version */


usbd_device *g_usbd_dev = 0;

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

static char usb_serial_number[25]; /* 12 bytes of desig and a \0 */

static const char * usb_strings[] = {
    "libopencm3.org",
    "AUDIO demo",
    usb_serial_number
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

#define WAVEFORM_SAMPLES 16

/* Samples interleaved L,R,L,R ==> actually samples/2 'time' samples */
int16_t waveform_data[WAVEFORM_SAMPLES] = {0};

void init_waveform_data()
{
    /* Just transmit a boring sawtooth waveform on both channels */
    for (int i = 0; i != WAVEFORM_SAMPLES/2; ++i) {
        waveform_data[i*2] = i*1024;
        waveform_data[i*2+1] = i*1024;
    }
}

/* HACK: upstream libopencm3 currently does not handle isochronous endpoints
 * correctly. We must program the USB peripheral with an even/odd frame bit,
 * toggling it so that we respond to every iso IN request from the host.
 * If this toggling is not performed, we only get half the bandwidth. */
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

void usbaudio_iso_stream_callback(usbd_device *usbd_dev, uint8_t ep)
{
    //gpio_clear(LED_GREEN_PORT, LED_GREEN_PIN);
    toggle_isochronous_frame(ep);
    usbd_ep_write_packet(usbd_dev, 0x82, waveform_data, WAVEFORM_SAMPLES*2);
}

static void usbaudio_set_config(usbd_device *usbd_dev, uint16_t wValue)
{
    (void)wValue;

    usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_ISOCHRONOUS, WAVEFORM_SAMPLES*2, usbaudio_iso_stream_callback);

    /* XXX: This is necessary -> not exactly sure why? */
    usbd_ep_write_packet(usbd_dev, 0x82, waveform_data, WAVEFORM_SAMPLES*2);
}

void otg_fs_isr(void)
{
    usbd_poll(g_usbd_dev);
}

void hard_fault_handler(void)
{
    while (1);
}

int main(void)
{
    rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);

    /*USB init*/
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_OTGFS);
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF10, GPIO11 | GPIO12);

    /* Set up both LEDs as outputs */
    rcc_periph_clock_enable(RCC_GPIOE);
    gpio_mode_setup(LED_RED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_RED_PIN);
    //gpio_mode_setup(LED_GREEN_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_GREEN_PIN);

    //gpio_set(LED_GREEN_PORT, LED_GREEN_PIN);

    init_waveform_data();

    /* Configure the USB core & stack */
    g_usbd_dev = usbd_init(&otgfs_usb_driver, &dev, &config,
                   usb_strings, 3, usbd_control_buffer,
                   sizeof(usbd_control_buffer));
    OTG_FS_GCCFG |= OTG_GCCFG_NOVBUSSENS | OTG_GCCFG_PWRDWN;
    OTG_FS_GCCFG &= ~(OTG_GCCFG_VBUSBSEN | OTG_GCCFG_VBUSASEN);
    usbd_register_set_config_callback(g_usbd_dev, usbaudio_set_config);

    /* Enable USB IRQs */
    nvic_enable_irq(NVIC_OTG_FS_IRQ);

    while (1) {
        gpio_toggle(LED_RED_PORT, LED_RED_PIN);
        //gpio_set(LED_GREEN_PORT, LED_GREEN_PIN);

        for (int i = 0; i != 200000; ++i)
            __asm__("nop");
    }
}
