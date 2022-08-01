#include "cbuf.h"
#define BUF_SIZE 256
int16_t ringdata[BUF_SIZE] = {0};
cbuf_s16 ring_buffer = {
	.buffer = ringdata,
	.head = 0,
	.tail = 0,
	.length = 0,
	.maxindex = BUF_SIZE -1
};


void toggle_isochronous_frame(uint8_t ep);
void endpoint_callback(usbd_device *usbd_dev, uint8_t ep);
void usbaudio_set_config(usbd_device *usbd_dev, uint16_t wValue);

