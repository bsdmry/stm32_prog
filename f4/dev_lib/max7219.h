#ifndef _H_MAX7219
#define _H_MAX7219

#define MAX7219_MODE_DECODE       0x09
#define MAX7219_MODE_INTENSITY    0x0A
#define MAX7219_MODE_SCAN_LIMIT   0x0B
#define MAX7219_MODE_POWER        0x0C
#define MAX7219_MODE_TEST         0x0F
#define MAX7219_MODE_NOOP         0x00

#define MAX7219_CHAR_BLANK        0xF
#define MAX7219_CHAR_NEGATIVE     0xA
#define MAX7219_CHAR_E     	  0xB

#define CHECK_FLASHING_BIT(x) (x & (1UL << 0) )
#define SET_FLASHING_BIT(x) (x |= (1UL << 0) )
#define CLEAR_FLASHING_BIT(x) (x &= ~(1UL << 0) )

#define CHECK_CODE_B_BIT(x) (x & (1UL << 1) )
#define SET_CODE_B_BIT(x) (x |= (1UL << 1) )
#define CLEAR_CODE_B_BIT(x) (x &= ~(1UL << 1) )

#define INSTANT_SHOW_ON 1
#define INSTANT_SHOW_OFF 0

#define THOUSAND_DOT_ON 1
#define THOUSAND_DOT_OFF 0

#define LEAD_ZEROS_ON 1
#define LEAD_ZEROS_OFF 0



typedef struct {
	uint8_t ctrlNumber;
	uint8_t* ledData;
	uint8_t* ledOptions;
	uint8_t blinkSwitch;
} max7219;


max7219* max7219_init(uint8_t chip_count);
void max7219_write_cfg_cmd(max7219* led, uint8_t reg, uint8_t data);
void max7219_set_flashing_bitset(max7219* led, uint8_t* bitsets, uint8_t mask_len);
void max7219_set_codeb_bitset(max7219* led, uint8_t* bitsets, uint8_t mask_len);
void max7219_display_data(max7219* led);
void max7219_clear(max7219* led);
uint8_t max7219_num2bitmap(uint8_t n);
uint8_t max7219_char2bitmap(char c);

void max7219_send_text(max7219* led, char* text, uint8_t start_index, uint8_t area_size, uint8_t instantUpdate);
void max7219_send_numeric(max7219* led, int32_t numeric, uint8_t start_index, uint8_t area_size, uint8_t zerolead_flag, uint8_t thousanddot_flag, uint8_t instantUpdate);

#endif
