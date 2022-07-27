#ifndef LCD1206_GPIO_H_
#define LCD1206_GPIO_H_
#include <stdint.h>
#define LCD1602_ENABLE 0x04
#define LCD1602_WIDTH 20   // Maximum characters per line
#define LCD1602_CHR  1 // Mode - Sending data
#define LCD1602_CMD  0 // Mode - Sending command
#define LCD1602_LINE_1  0x80 // LCD RAM address for the 1st line
#define LCD1602_LINE_2  0xC0 // LCD RAM address for the 2nd lin
#define RS_PIN GPIO15
#define RW_PIN GPIO14
#define E_PIN GPIO13
#define DATA0 GPIO11
#define DATA1 GPIO10
#define DATA2 GPIO9
#define DATA3 GPIO8
void lcd1602_gpio_setup(void);
void set_nibble(uint8_t byte);
void lcd1602_gpio_write_byte(uint8_t byte, uint8_t flag);
void lcd1602_gpio_print (unsigned char *msg, uint8_t num,  uint8_t line);
void lcd1602_gpio_print_offset (unsigned char *msg, uint8_t num,  uint8_t line, uint8_t offset);
void lcd1602_gpio_init(void);
#endif /* LCD1206_GPIO_H_ */
