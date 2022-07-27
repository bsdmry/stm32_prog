#ifndef F4_USART_H
#define F4_USART_H
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/gpio.h>
#include <stdlib.h>

#define USART1_A_PORT_RCC RCC_GPIOA
#define USART1_A_PORT GPIOA
#define USART1_A_RX GPIO10
#define USART1_A_TX GPIO9

#define USART2_A_PORT_RCC RCC_GPIOA
#define USART2_A_PORT GPIOA
#define USART2_A_RX GPIO3
#define USART2_A_TX GPIO2

void usart_setup( uint32_t usart, uint32_t baud, uint32_t databits, uint32_t stopbits, uint32_t mode, uint32_t parity, uint32_t flowcontrol);
void usart_send_string(uint32_t usart, char* pStr, uint16_t len);

#endif
