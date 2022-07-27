#include <stdint.h>
void usartWriteString(uint8_t *strBuf, uint8_t len);
void usart_init(void);
void usart_isr(void);
void usart_clock_setup(void);
void usart_setup(void);
void putToRxBuf(uint8_t data);

