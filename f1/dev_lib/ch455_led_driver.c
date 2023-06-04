#include "ch455_led_driver.h"
#include "dwt.h"

ch455* ch455_init(uint32_t port, uint32_t sda_pin, uint32_t scl_pin){
	gpio_set_mode(port, GPIO_MODE_OUTPUT_10_MHZ,  GPIO_CNF_OUTPUT_PUSHPULL, sda_pin | scl_pin);
	gpio_set(port, sda_pin | scl_pin);
	ch455* led = malloc(sizeof(ch455));
	led->ledData = calloc(4, sizeof(uint8_t));
	led->ledOptions = calloc(4, sizeof(uint8_t));
	led->brightness = 7;
	led->sda_pin = sda_pin;
	led->scl_pin = scl_pin;
	led->port = port;
	led->blinkSwitch = 0;
	return led;
}

void ch455_send_bit(ch455* led, uint8_t bit){
	gpio_clear(led->port, led->scl_pin);
	dwt_delay_us(1);
	if(bit){
		gpio_set(led->port, led->sda_pin);
	} else {
		gpio_clear(led->port, led->sda_pin);
	}
	gpio_set(led->port, led->scl_pin);
	dwt_delay_us(2);
	gpio_clear(led->port, led->scl_pin);
}

void ch455_write_cfg(ch455* led){
	//bit 0 - ENABLE bit
	//bit 1 - Not used
	//bit 2 - SLEEP bit
	//bit 3 - "1" - 7 segment mode. "dp" pin acts as INT
	//        "0" - normal 8 segment mode
	//bit 4:6 - brightnes level 1-7
	//bit 7 - "1" only LED driver in use
	//	  "0" LED
	//uint8_t cfg = 0b10000001;
	uint8_t cfg = 0b00000001;
	cfg = (cfg | (led->brightness << 4));
	ch455_write(led, CFG_REG, cfg);	
}

void ch455_set_brightness(ch455* led, uint8_t level){
	if (level > 7){
		led->brightness = 7;
	} else {
		if (level == 0){
			led->brightness = 1;
		}else{
			led->brightness = level;
		}
	}
}

void ch455_write(ch455* led, uint8_t cmd, uint8_t data){
	gpio_clear(led->port, led->sda_pin);//START 
	for(uint8_t i=0; i<9; i++){
		if (i == 8){
			ch455_send_bit(led, 1);
		} else {
                	ch455_send_bit(led, (cmd<<i)&0x80);
		}
	}
	for(uint8_t i=0; i<9; i++){
		if (i == 8){
			ch455_send_bit(led, 1);
		} else {
                	ch455_send_bit(led, (data<<i)&0x80);
		}
	}
	gpio_set(led->port, led->scl_pin);
	dwt_delay_us(20);
	gpio_clear(led->port, led->sda_pin);//STOP
	dwt_delay_us(20);
	gpio_set(led->port, led->sda_pin);
}

void ch455_send_number(ch455* led, uint16_t num, uint8_t pos, uint8_t len,  uint8_t instantUpdate){
	uint8_t dig;
	for (uint8_t p = 0; p < len; p++) {
		dig = num % 10;
		led->ledData[p + pos] = ch455_char2bitmap((char)(dig + 48));
		num /= 10;
	}
	if (instantUpdate) {
		ch455_display_data(led);
	}
}

void ch455_send_fraction(ch455* led, uint16_t int_part, uint8_t int_pos, uint8_t int_len,  
		uint16_t frac_part, uint8_t frac_pos, uint8_t frac_len, uint8_t instantUpdate){
	uint8_t dig;
	for (uint8_t p = 0; p < int_len; p++) {
		dig = int_part % 10;
		led->ledData[p + int_pos] = ch455_char2bitmap((char)(dig + 48));
		if (p == 0) {
			led->ledData[p + int_pos] |= 1 << 7; 
		}
		int_part /= 10;
	}
	for (uint8_t p = 0; p < frac_len; p++) {
		dig = frac_part % 10;
		led->ledData[p + frac_pos] = ch455_char2bitmap((char)(dig + 48));
		frac_part /= 10;
	}

	if (instantUpdate) {
		ch455_display_data(led);
	}
}

void ch455_send_text(ch455* led, char* text, uint8_t pos, uint8_t instantUpdate){
	uint8_t strLen = strlen(text);
	uint8_t dotCount = 0;
	for (uint8_t c = 0; c < strLen; c++){
		if ((text[c] == ',') || (text[c] == '.')){
			dotCount++;
		}
	}
	if ((strLen - dotCount) > 4){
		strLen = 4;
	}
	uint8_t charPointer = strLen;
	uint8_t nextDotFlag = 0;
	uint8_t ledCharPointer= pos;
	while(charPointer > 0){
		if ((text[charPointer-1] == '.') || (text[charPointer-1] == ',')){
			nextDotFlag = 1;
		} else {
			uint8_t bitmap = ch455_char2bitmap(text[charPointer-1]);
			if (nextDotFlag){
				bitmap |= 1 << 7;
				nextDotFlag = 0;
			}
			led->ledData[ledCharPointer] = bitmap;
			ledCharPointer++;
		}
		charPointer--;
	}
	if (instantUpdate){
		ch455_display_data(led);
	}
}

/** @brief Sends data from a buffer to an indicator
 
  This function send data to a LED indicator. You
  can put call the function to a timer interrupt 
  routine for automatical refresh. If you also toggle
  led->blinkSwitch faster then calling this function
  it will allow to use the flashing functionality

 @param [in] max7219* led Control object for CH455 chip
 */
void ch455_display_data(ch455* led){
	uint8_t pos_addresses[4] = {DIG3_REG, DIG2_REG, DIG1_REG, DIG0_REG}; 
	for (uint8_t p = 0; p < 4; p++){
		uint16_t tx = 0;
		if (CHECK_FLASHING_BIT(led->ledOptions[p])){
			if (led->blinkSwitch){
				tx = led->ledData[p];	
			} else {
				tx = 0;
			}
		} else {
			tx = led->ledData[p];
		}
		ch455_write(led, pos_addresses[p], tx);
	}
}

void ch455_set_flashing_bitset(ch455* led, uint8_t bitset){
	for (uint8_t i =0; i< 4; i++){
        	if ((bitset >> i) & 0x01){
                	SET_FLASHING_BIT(led->ledOptions[i]);
                } else {
                       CLEAR_FLASHING_BIT(led->ledOptions[i]);
                }
	}
}

/** @brief Translates an ASCII char to a LED bitmap
 

 @param [in] char c Char to display
 @returns data uint8_t Bitmap for a 7-segment display
 */
uint8_t ch455_char2bitmap(char c){
	uint8_t val = 0;
	switch(c){
		case '0': val = 0b00111111; break;
		case '1': val = 0b00000110; break;
		case '2': val = 0b01011011; break;
		case '3': val = 0b01001111; break;
		case '4': val = 0b01100110; break;
		case '5': val = 0b01101101; break;
		case '6': val = 0b01111101; break;
		case '7': val = 0b00000111; break;
		case '8': val = 0b01111111; break;
		case '9': val = 0b01101111; break;

		case 'A': val = 0b01110111; break;
		case 'a': val = 0b01110111; break;
		case 'B': val = 0b01111100; break;
		case 'b': val = 0b01111100; break;
		case 'C': val = 0b00111001; break;
		case 'c': val = 0b00111001; break;
		case 'D': val = 0b01011110; break;
		case 'd': val = 0b01011110; break;
		case 'E': val = 0b01111001; break;
		case 'F': val = 0b01110001; break;
		case 'G': val = 0b00111101; break;
		case 'H': val = 0b01110110; break;
		case 'h': val = 0b01110100; break;
		case 'J': val = 0b00011110; break;
		case 'L': val = 0b00111000; break;
		case 'O': val = 0b00111111; break;
		case 'o': val = 0b01011100; break;
		case 'P': val = 0b01110011; break;
		case 'r': val = 0b01010000; break;
		case 'S': val = 0b01101101; break;
		case 't': val = 0b01111000; break;
		case 'q': val = 0b01100111; break;
		case 'U': val = 0b00111110; break;
		case 'Y': val = 0b00111110; break;//
		case '-': val = 0b01000000; break;
		case '_': val = 0b00001000; break;
		case '=': val = 0b01001000; break;

		default: val = 0; break;
	}
	return val;
}

