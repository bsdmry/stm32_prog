#include <stdint.h>
void lcd1602_i2c_setup(void);
void lcd1602_i2c_write(uint8_t addr, uint8_t data);
void lcd1602_write_byte(uint8_t byte, uint8_t flag);
void lcd1602_print (unsigned char *msg, uint8_t num,  uint8_t pos);
void lcd1602_init(void);
