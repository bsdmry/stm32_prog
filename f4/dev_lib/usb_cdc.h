#ifndef USB_CDC_H
#define USB_CDC_H

#include <stdlib.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/usb/dwc/otg_fs.h>
#include <libopencm3/usb/dwc/otg_common.h>
#include "cbuf.h"
#define MAX_PACKET_SIZE 64
#define TRX_BUFFER_SIZE 128

extern const struct usb_device_descriptor dev;
extern const struct usb_endpoint_descriptor comm_endp[];
extern const struct usb_endpoint_descriptor data_endp[];
struct func_descriptors{
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed));

extern const struct usb_interface_descriptor comm_iface[];
extern const struct usb_interface_descriptor data_iface[];
extern const struct usb_interface ifaces[];
extern const struct usb_config_descriptor config;

extern usbd_device *usbd_dev_g;
extern uint8_t usb_connected;

extern cbuf_u8 tx_ring_buffer;
extern cbuf_u8 rx_ring_buffer;
static uint8_t cdc_rx_buffer_has_newline;

enum usbd_request_return_codes cdcacm_control_request(usbd_device *usbd_dev,
	struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
	void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req));
void usb_disconnect_cb(void);
void cdcacm_data_rx_cb(usbd_device *usbd_dev, uint8_t ep);
void cdcacm_data_tx(usbd_device *usbd_dev);
void cdcacm_set_config(usbd_device *usbd_dev, uint16_t wValue);
void init_usb_cdc(uint8_t enable_echo);
void cdc_print(char* str);
void cdc_send(uint8_t* data, uint16_t len);

#endif


