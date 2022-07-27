void toggle_isochronous_frame(uint8_t ep);
void endpoint_callback(usbd_device *usbd_dev, uint8_t ep);
void usbaudio_set_config(usbd_device *usbd_dev, uint16_t wValue);

