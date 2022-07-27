#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <string.h>
#include "usart_cfg.h"
#define TX_BUF_LEN 32
#define RX_BUF_LEN 32

uint8_t rx_buf[RX_BUF_LEN];
uint8_t rx_buf_index = 0;
uint8_t rx_data_len = 0;
uint8_t rx_buf_ready = 0;
uint8_t tx_buf[TX_BUF_LEN] = {0,};
uint8_t tx_data_len = 0;

void usart_clock_setup(void)
{
    /* Enable GPIOA clock for USARTs. */
    rcc_periph_clock_enable(RCC_GPIOA);
    /* Enable clocks for USART1. */
    rcc_periph_clock_enable(RCC_USART1);
}

void usart_setup(void)
{
    /* Enable the USART1 interrupt. */
    nvic_enable_irq(NVIC_USART1_IRQ);

    /* Setup GPIO pins for USART1 transmit. */
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);

    /* Setup GPIO pins for USART1 receive. */
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10);
    gpio_set_output_options(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, GPIO10);

    /* Setup USART1 TX and RX pin as alternate function. */
    gpio_set_af(GPIOA, GPIO_AF7, GPIO9);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO10);

    /* Setup USART1 parameters. */
    usart_set_baudrate(USART1, 9600);
    usart_set_databits(USART1, 8);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    usart_set_mode(USART1, USART_MODE_TX_RX);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

    /* Enable USART1 Receive interrupt. */
    usart_enable_rx_interrupt(USART1);

    /* Finally enable the USART. */
    usart_enable(USART1);
}

void putToRxBuf(uint8_t data){
    rx_buf[rx_buf_index] = data;
    rx_data_len = rx_buf_index;
    rx_buf_index++;
    if (rx_buf_index >= RX_BUF_LEN){
        rx_buf_index = 0;
    }
    if (data == 0x0a){
        rx_buf_ready = 1;
    }
}

void usartWriteString(uint8_t *strBuf, uint8_t len){
    if (len <= TX_BUF_LEN -1){
        memcpy(tx_buf, strBuf, len);
        tx_buf[len] = 0x0d;
        tx_buf[len+1] = 0x0a;
        tx_data_len = len + 2;
        usart_enable_tx_interrupt(USART1); 
    }
}

void usart_init(void){
    usart_clock_setup();
    usart_setup();
}

void usart1_isr(void)
{
    static uint8_t data = 'L';

    /* Check if we were called because of RXNE. */
    if (((USART_CR1(USART1) & USART_CR1_RXNEIE) != 0) &&
        ((USART_SR(USART1) & USART_SR_RXNE) != 0)) {
        /* Retrieve the data from the peripheral. */
        data = usart_recv(USART1);
        putToRxBuf(data);
    }

    /* Check if we were called because of TXE. */
    if (((USART_CR1(USART1) & USART_CR1_TXEIE) != 0) &&
        ((USART_SR(USART1) & USART_SR_TXE) != 0)) {
        for (uint8_t i = 0; i < tx_data_len; i++){
           usart_send_blocking(USART1, tx_buf[i]);
        }
       tx_data_len = 0;
        /* Disable the TXE interrupt as we don't need it anymore. */
        usart_disable_tx_interrupt(USART1);
    }
}
