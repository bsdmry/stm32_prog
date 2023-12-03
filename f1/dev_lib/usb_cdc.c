#include "usb_cdc.h"

usbd_device *usbd_dev_g = 0;
uint8_t usb_connected = 0;
uint8_t cdc_enable_echo_flag = 0;
static uint8_t cdc_tx_buffer[TRX_BUFFER_SIZE] = {0};
static uint8_t cdc_rx_buffer[TRX_BUFFER_SIZE] = {0};
static uint8_t cdc_rx_buffer_has_newline = 0;

static struct usb_cdc_line_coding *line_coding;

rb_u8 tx_ring_buffer = {
	.data = cdc_tx_buffer,
  	.size = sizeof(cdc_tx_buffer),
  	.head = 0,
  	.tail = 0,
  	.length = 0	
};

rb_u8 rx_ring_buffer = {
	.data = cdc_rx_buffer,
  	.size = sizeof(cdc_rx_buffer),
  	.head = 0,
  	.tail = 0,
  	.length = 0	
};

const struct usb_device_descriptor dev = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = USB_CLASS_CDC,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x0483,
	.idProduct = 0x5740,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

const struct usb_endpoint_descriptor comm_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x83,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 16,
	.bInterval = 255,
} };

const struct usb_endpoint_descriptor data_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 1,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x82,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 1,
} };

const struct func_descriptors cdcacm_functional_descriptors = {
	.header = {
		.bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_HEADER,
		.bcdCDC = 0x0110,
	},
	.call_mgmt = {
		.bFunctionLength =
			sizeof(struct usb_cdc_call_management_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
		.bmCapabilities = 0,
		.bDataInterface = 1,
	},
	.acm = {
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_ACM,
		.bmCapabilities = 0,
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 0,
		.bSubordinateInterface0 = 1,
	 }
};

const struct usb_interface_descriptor comm_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface = 0,

	.endpoint = comm_endp,

	.extra = &cdcacm_functional_descriptors,
	.extralen = sizeof(cdcacm_functional_descriptors)
} };

const struct usb_interface_descriptor data_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 1,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = data_endp,
} };

const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = comm_iface,
}, {
	.num_altsetting = 1,
	.altsetting = data_iface,
} };


const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 2,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,
	.bMaxPower = 0x32,

	.interface = ifaces,
};

static const char * usb_strings[] = {
	"Black Sphere Technologies",
	"CDC-ACM",
	"0001",
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

enum usbd_request_return_codes cdcacm_control_request(
		usbd_device *usbd_dev,
		struct usb_setup_data *req, 
		uint8_t **buf, 
		uint16_t *len,
		void (**complete)(
			usbd_device *usbd_dev, 
			struct usb_setup_data *req
		)
)
{
	(void)complete;
	(void)buf;
	(void)usbd_dev;

	switch (req->bRequest) {
	case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
		/*
		 * This Linux cdc_acm driver requires this to be implemented
		 * even though it's optional in the CDC spec, and we don't
		 * advertise it in the ACM functional descriptor.
		 */
		return USBD_REQ_HANDLED;
		}
	case USB_CDC_REQ_SET_LINE_CODING:
		if (*len < sizeof(struct usb_cdc_line_coding)) {
			return USBD_REQ_NOTSUPP;
		}
		//*line_coding = (struct usb_cdc_line_coding *)&buf;
		line_coding = (struct usb_cdc_line_coding *)*buf;
		get_cdc_comm_config(
				line_coding->dwDTERate, 
				line_coding->bCharFormat,
			       	line_coding->bParityType,
			       	line_coding->bDataBits
		);
		return USBD_REQ_HANDLED;
	}
	return USBD_REQ_NOTSUPP;
}


void cdcacm_data_rx_cb(usbd_device *usbd_dev, uint8_t ep)
{
	(void)ep;

	char buf[MAX_PACKET_SIZE];
	int len = usbd_ep_read_packet(usbd_dev, 0x01, buf, MAX_PACKET_SIZE);

	if (len) {
		for (int i = 0; i < len; i++){
			if (buf[i] == 0x0A) {cdc_rx_buffer_has_newline = 1; }
			rb_u8_push(&rx_ring_buffer, (uint8_t)buf[i]);
		}
		if (cdc_enable_echo_flag) {
			while (usbd_ep_write_packet(usbd_dev, 0x82, buf, len) == 0);
		}
	}
}

void cdcacm_data_tx(usbd_device *usbd_dev){
	if (usb_connected){
		unsigned char buf[MAX_PACKET_SIZE];
		uint8_t i;
		while (tx_ring_buffer.length > 0) {
			for (i = 0; i < MAX_PACKET_SIZE; i++){
				uint8_t res = rb_u8_pop(&tx_ring_buffer, &buf[i]);
				if (res == 0) { break; }
			}
			while (usbd_ep_stall_get(usbd_dev, 0x82)) {};
			usbd_ep_write_packet(usbd_dev, 0x82, buf, i);
		}
	}
}

void cdc_print(char* str){
	if (usb_connected){
		for (uint16_t i = 0; i < 65535; i++){
			if (str[i] == 0) { break; } else { rb_u8_push(&tx_ring_buffer, (uint8_t)str[i]);}
		}
		cdcacm_data_tx(usbd_dev_g);
	}
}

void cdc_send(uint8_t* data, uint16_t len){
	if (usb_connected){
		for (uint16_t i = 0; i < len; i++){
			rb_u8_push(&tx_ring_buffer, data[i]);
		}
	}
}

void cdc_dbg_print(char* msg, uint32_t data){
	uint32_t len = strlen(msg);
	uint32_t slen = len + 1 + 10 + 2 + 1;  //text len + space + number + \r\n + NULL
	char *str = (char *)malloc(sizeof(char)*slen);
	memset(str, 0x20, slen);
	memcpy(str, msg, len);
	str[len+1] = '0';
	str[len+2] = 'x';
	char data_s[11] = {0x20};
	itoa(data, data_s, 16);
	uint16_t data_s_len = strlen(data_s);
	memcpy(str+len+3, data_s, data_s_len);
	str[slen-3] = '\r';
	str[slen-2] = '\n';
	str[slen-1] = 0;
	cdc_print(str);
	free(str);
}

void usb_disconnect_cb(void){
	usb_connected = 0;
}

void cdcacm_set_config(usbd_device *usbd_dev, uint16_t wValue)
{
	(void)wValue;

	usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, MAX_PACKET_SIZE, cdcacm_data_rx_cb);
	usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_BULK, MAX_PACKET_SIZE, NULL);
	usbd_ep_setup(usbd_dev, 0x83, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

	usbd_register_control_callback(
				usbd_dev,
				USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
				USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
				cdcacm_control_request);
	usb_connected = 1;
	usbd_register_suspend_callback(usbd_dev, usb_disconnect_cb);
}

void usb_lp_can_rx0_isr(void)
{
    usbd_poll(usbd_dev_g);
}

void usb_dataline_rst(void){
	rcc_periph_clock_enable(RCC_GPIOA);
	gpio_set_mode (GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
	gpio_clear (GPIOA, GPIO12);
        for (int i = 0; i < 0x8000; ++i){
        	__asm("nop");
	}
}
//https://github.com/roddehugo/bluepill-serial/blob/master/src/cdcacm.c#L9
void init_usb_cdc(uint8_t enable_echo){
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_USB);
	
	usb_dataline_rst();
	
	usbd_dev_g = usbd_init(&st_usbfs_v1_usb_driver, &dev, &config,
			usb_strings, 3,
			usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(usbd_dev_g, cdcacm_set_config);
	cdc_enable_echo_flag = enable_echo;
	/* Enable USB IRQs */
    	nvic_enable_irq(NVIC_USB_LP_CAN_RX0_IRQ);
	nvic_set_priority(NVIC_USB_LP_CAN_RX0_IRQ, 1);
}


