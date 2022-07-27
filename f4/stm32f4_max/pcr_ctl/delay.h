#ifndef DELAY_H_
#define DELAY_H_
#include <stdint.h>
void enable_dwt_counter(void);
uint32_t delta(uint32_t t0, uint32_t t1);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

#endif /* DELAY_H_ */
