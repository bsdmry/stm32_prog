#include "m62429.h"
#include "dwt.h"

m62429* m62429_init(uint32_t port, uint32_t sda_pin, uint32_t scl_pin){
	gpio_set_mode(port, GPIO_MODE_OUTPUT_10_MHZ,  GPIO_CNF_OUTPUT_PUSHPULL, sda_pin | scl_pin);
	gpio_set(port, sda_pin | scl_pin);
	m62429* sw = malloc(sizeof(m62429));
	sw->data_pin = sda_pin;
	sw->clock_pin = scl_pin;
	sw->port = port;

	return sw;
}


void m62429_send_bit(m62429* sw, uint8_t bit){
	dwt_delay_us(10);
	gpio_clear(sw->port, sw->data_pin);
	dwt_delay_us(10);
	gpio_clear(sw->port, sw->clock_pin);
	dwt_delay_us(10);
	if(bit){
		gpio_set(sw->port, sw->data_pin);
	} else {
		gpio_clear(sw->port, sw->data_pin);
	}
	dwt_delay_us(10);
	gpio_set(sw->port, sw->clock_pin);
}

void m62429_set_volume(m62429* sw, uint8_t channel, uint8_t volume){
	if (channel){
		sw->ch2_volume = volume;
	} else {
		sw->ch1_volume = volume;
	}
	uint16_t tx_data = 0;
	volume = (volume > 100) ? 0 : (((volume * 83) / -100) + 83);
	tx_data |= (channel << 0); // D0 (channel select: 0=ch1, 1=ch2)	
	tx_data |= (1 << 1); // D1 (individual/both select: 0=both, 1=individual)
	tx_data |= ((21 - (volume / 4)) << 2); // D2...D6 (ATT1: coarse attenuator: 0,-4dB,-8dB, etc.. steps of 4dB)
	tx_data |= ((3 - (volume % 4)) << 7); // D7...D8 (ATT2: fine attenuator: 0...-1dB... steps of 1dB)
	tx_data |= (0b11 << 9); 
	
	gpio_clear(sw->port, sw->clock_pin);
	gpio_clear(sw->port, sw->data_pin);
	for(uint8_t i=0; i<11; i++){
        	m62429_send_bit(sw, (tx_data >> i) & 0x01);	
	}
	dwt_delay_us(10);
	gpio_set(sw->port, sw->data_pin);
	dwt_delay_us(10);
	gpio_clear(sw->port, sw->clock_pin);	
}
