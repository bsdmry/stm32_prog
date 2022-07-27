#include "spi.h"

void ad985x_init(uint32_t spi, uint32_t port_rst, uint16_t pin_rst);
void ad985x_reset(uint32_t port_rst, uint16_t pin_rst);
void ad985x_set_freq(uint32_t spi, uint32_t crystal_frequency, double frequency);
void ad9850_set_freq(uint32_t spi, double frequency);
void ad9851_set_freq(uint32_t spi, double frequency);
