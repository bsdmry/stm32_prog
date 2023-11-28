#ifndef F1_SPI_H
#define F1_SPI_H
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/gpio.h>
#include <stdlib.h>
#define SPI1_A_PORT_RCC RCC_GPIOA
#define SPI1_A_PORT GPIOA
#define SPI1_A_MISO GPIO6
#define SPI1_A_MOSI GPIO7
#define SPI1_A_SCK GPIO5
#define SPI1_A_CS GPIO4

#define SPI2_A_PORT_RCC RCC_GPIOB
#define SPI2_A_PORT GPIOB
#define SPI2_A_MISO GPIO14
#define SPI2_A_MOSI GPIO15
#define SPI2_A_SCK GPIO13
#define SPI2_A_CS GPIO12

void spi_setup(uint32_t spi, uint32_t br, uint32_t cpol, uint32_t cpha, uint32_t dff, uint32_t lsbfirst );
void spi_tx_array16(int32_t spi, uint16_t* data, uint16_t len);
void spi_tx_array16_nocs(int32_t spi, uint16_t* data, uint16_t len);
void spi_tx_array8(int32_t spi, uint8_t* data, uint16_t len);
void spi_tx(int32_t spi, uint16_t data);
void spi_tx_nocs(int32_t spi, uint16_t data);
void spi_tx8(int32_t spi, uint8_t data);
uint8_t spi_rx_reg8(int32_t spi, uint8_t reg);
void spi_set_cs(int32_t spi, uint8_t state);

#endif
