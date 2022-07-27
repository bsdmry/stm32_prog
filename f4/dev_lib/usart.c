#include "usart.h"

void usart_setup(
	uint32_t usart, 
	uint32_t baud, 
	uint32_t databits, 
	uint32_t stopbits, 
	uint32_t mode, 
	uint32_t parity, 
	uint32_t flowcontrol
	){
	if (usart == USART1){
		rcc_periph_clock_enable(USART1_A_PORT_RCC);
		rcc_periph_clock_enable(RCC_USART1);
		switch(mode){
			case USART_MODE_RX:
        			gpio_mode_setup(USART1_A_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART1_A_RX);
        			gpio_set_af(USART1_A_PORT, GPIO_AF7, USART1_A_RX);
				break;
			case USART_MODE_TX:
        			gpio_mode_setup(USART1_A_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART1_A_TX);
        			gpio_set_af(USART1_A_PORT, GPIO_AF7, USART1_A_TX);
				break;
			case USART_MODE_TX_RX:
        			gpio_mode_setup(USART1_A_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART1_A_TX | USART1_A_RX);
        			gpio_set_af(USART1_A_PORT, GPIO_AF7, USART1_A_TX | USART1_A_RX);
				break;
			default:
        			gpio_mode_setup(USART1_A_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART1_A_TX | USART1_A_RX);
        			gpio_set_af(USART1_A_PORT, GPIO_AF7, USART1_A_TX | USART1_A_RX);
					
		}
	} else if (usart == USART2){
		rcc_periph_clock_enable(USART2_A_PORT_RCC);
		rcc_periph_clock_enable(RCC_USART2);
		switch(mode){
			case USART_MODE_RX:
        			gpio_mode_setup(USART2_A_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART2_A_RX);
        			gpio_set_af(USART2_A_PORT, GPIO_AF7, USART2_A_RX);
				break;
			case USART_MODE_TX:
        			gpio_mode_setup(USART2_A_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART2_A_TX);
        			gpio_set_af(USART2_A_PORT, GPIO_AF7, USART2_A_TX);
				break;
			case USART_MODE_TX_RX:
        			gpio_mode_setup(USART2_A_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART2_A_TX | USART2_A_RX);
        			gpio_set_af(USART2_A_PORT, GPIO_AF7, USART2_A_TX | USART2_A_RX);
				break;
			default:
        			gpio_mode_setup(USART2_A_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART2_A_TX | USART2_A_RX);
        			gpio_set_af(USART2_A_PORT, GPIO_AF7, USART2_A_TX | USART2_A_RX);
					
		}

	} else {
		rcc_periph_clock_enable(USART1_A_PORT_RCC);
		rcc_periph_clock_enable(RCC_USART1);
		switch(mode){
			case USART_MODE_RX:
        			gpio_mode_setup(USART1_A_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART1_A_RX);
        			gpio_set_af(USART1_A_PORT, GPIO_AF7, USART1_A_RX);
				break;
			case USART_MODE_TX:
        			gpio_mode_setup(USART1_A_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART1_A_TX);
        			gpio_set_af(USART1_A_PORT, GPIO_AF7, USART1_A_TX);
				break;
			case USART_MODE_TX_RX:
        			gpio_mode_setup(USART1_A_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART1_A_TX | USART1_A_RX);
        			gpio_set_af(USART1_A_PORT, GPIO_AF7, USART1_A_TX | USART1_A_RX);
				break;
			default:
        			gpio_mode_setup(USART1_A_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, USART1_A_TX | USART1_A_RX);
        			gpio_set_af(USART1_A_PORT, GPIO_AF7, USART1_A_TX | USART1_A_RX);				
		}
	}
        /* Setup USART2 parameters. */
        usart_set_baudrate(usart, baud);
        usart_set_databits(usart, databits);
        usart_set_stopbits(usart, stopbits);
        usart_set_mode(usart, mode);
        usart_set_parity(usart, parity);
        usart_set_flow_control(usart, flowcontrol);

        /* Finally enable the USART. */
        usart_enable(usart);
}

void usart_send_string(uint32_t usart, char* pStr, uint16_t len){
        for (uint16_t i = 0; i < len; i++){
                usart_send_blocking(usart, (uint16_t)pStr[i]);
        }
}

