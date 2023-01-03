#include <string.h>
#include "spi.h"
#include "max7219.h"

/** @brief Init MAX7219 devices as a control object
 
  Functions inits all chips for no Code B, and with
  scan limit 7

 @param [in] uint8_t chip_count Number of connected MAX7219 chips
 @returns data max7219 Control object
 */
max7219* max7219_init(uint8_t chip_count){
	spi_setup(SPI2, SPI_CR1_BAUDRATE_FPCLK_DIV_64, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE, SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_16BIT, SPI_CR1_MSBFIRST);
	max7219* led = malloc(sizeof(max7219));
	led->ctrlNumber = chip_count;
	led->ledData = calloc(8*chip_count, sizeof(uint8_t));
	led->ledOptions = calloc(8*chip_count, sizeof(uint8_t));

	max7219_write_cfg_cmd(led, MAX7219_MODE_DECODE, 0x0); //Free byte
	max7219_write_cfg_cmd(led, MAX7219_MODE_SCAN_LIMIT, 7); //Max index
	max7219_write_cfg_cmd(led, MAX7219_MODE_INTENSITY, 8);
	max7219_write_cfg_cmd(led, MAX7219_MODE_POWER, 1);
	return led;
}

/** @brief Writes to configuration register for all chips
 
 @param [in] max7219* led Control object for MAX7219 chips
 @param [in] uint8_t reg Register
 @param [in] uint8_t data Value
 */
void max7219_write_cfg_cmd(max7219* led, uint8_t reg, uint8_t data){
        while (!(SPI_SR(SPI2) & SPI_SR_TXE));
	spi_set_cs(SPI2, 0);
	for (uint8_t i = led->ctrlNumber; i != 0; i--){
		uint16_t tx = 0;
		tx = reg;
		tx = (tx << 8) | data;
		spi_tx_nocs(SPI2, tx);
	}
	spi_set_cs(SPI2, 1);
}

/** @brief Setups bits for flashing
 
  Setups bitset for flashing. Each byte in the @ref bitsets
  represents single MAX7219 chip, and each bit in the byte
  represents number place

 @param [in] max7219* led Control object for MAX7219 chips
 @param [in] uint8_t* bitsets Bitfields
 @param [in] uint8_t mask_len Bitfields count
 */
void max7219_set_flashing_bitset(max7219* led, uint8_t* bitsets, uint8_t mask_len){
	for (uint8_t c = 0; c < mask_len; c++){
		for (uint8_t i =0; i< 8; i++){
			if ((bitsets[c] >> i) & 0x01){
				SET_FLASHING_BIT(led->ledOptions[c*8 + i]);
			} else {
				CLEAR_FLASHING_BIT(led->ledOptions[c*8 + i]);	
			}
		}
	}
}

/** @brief Setups bits for Code B
 
  Setups bitset for interpretation data for displaying as 
  Code B. Each byte in the @ref bitsets
  represents single MAX7219 chip, and each bit in the byte
  represents number place

 @param [in] max7219* led Control object for MAX7219 chips
 @param [in] uint8_t* bitsets Bitfields
 @param [in] uint8_t mask_len Bitfields count
 */
void max7219_set_codeb_bitset(max7219* led, uint8_t* bitsets, uint8_t mask_len){
	for (uint8_t c = 0; c < mask_len; c++){
		for (uint8_t i =0; i< 8; i++){
			if ((bitsets[c] >> i) & 0x01){
				SET_CODE_B_BIT(led->ledOptions[c*8 + i]);
			} else {
				CLEAR_CODE_B_BIT(led->ledOptions[c*8 + i]);	
			}
		}
	}
}

/** @brief Sends data from a buffer to an indicator
 
  This function send data to a LED indicator. You
  can put call the function to a timer interrupt 
  routine for automatical refresh. If you also toggle
  led->blinkSwitch faster then calling this function
  it will allow to use the flashing functionality

 @param [in] max7219* led Control object for MAX7219 chips
 */
void max7219_display_data(max7219* led){
	for (uint8_t p = 0; p < 8; p++){
		spi_set_cs(SPI2, 0);
		for (int8_t i = led->ctrlNumber-1; i >= 0; i--){
			uint16_t tx = 0;
			tx = p+1; //Position index for MAX7219 starts from 1
			uint8_t rindx = i*8 + p;
			if (CHECK_FLASHING_BIT(led->ledOptions[rindx])){
				if (led->blinkSwitch){
					tx = (tx << 8) | led->ledData[rindx];	
				} else {
					tx = (tx << 8);
				}
			} else {
				tx = (tx << 8) | led->ledData[rindx];
			}
			spi_tx_nocs(SPI2, tx);
		}
		spi_set_cs(SPI2, 1);
	}
}

/** @brief Clears a buffer and refreshes an indicator
 
 @param [in] max7219* led Control object for MAX7219 chips
 */

void max7219_clear(max7219* led){
	for (uint8_t i=0; i< led->ctrlNumber *8; i++){
		if (CHECK_CODE_B_BIT(led->ledOptions[i])){
			led->ledData[i] = MAX7219_CHAR_BLANK;
		} else {
			led->ledData[i] = 0;	
		}
	}	
	max7219_display_data(led);
}


/** @brief Translates a digit to a LED-bitmap
 
 @param [in] uint8_t n Digit to display
 @returns data uint8_t Bitmap for a 7-segment display
 */
uint8_t max7219_num2bitmap(uint8_t n){
	uint8_t val = 0;
	switch(n){
		case 0: val = 0b01111110 ; break;
		case 1: val = 0b00110000; break;
		case 2: val = 0b01101101; break;
		case 3: val = 0b01111001; break;
		case 4: val = 0b00110011; break;
		case 5: val = 0b01011011; break;
		case 6: val = 0b01011111; break;
		case 7: val = 0b01110000; break;
		case 8: val = 0b01111111; break;
		case 9: val = 0b01111011; break;
		default: val = 0; break;
	}
	return val;
}

/** @brief Translates an ASCII char to a LED bitmap
 

 @param [in] char c Char to display
 @returns data uint8_t Bitmap for a 7-segment display
 */
uint8_t max7219_char2bitmap(char c){
	uint8_t val = 0;
	switch(c){
		case '0': val = 0b01111110 ; break;
		case '1': val = 0b00110000; break;
		case '2': val = 0b01101101; break;
		case '3': val = 0b01111001; break;
		case '4': val = 0b00110011; break;
		case '5': val = 0b01011011; break;
		case '6': val = 0b01011111; break;
		case '7': val = 0b01110000; break;
		case '8': val = 0b01111111; break;
		case '9': val = 0b01111011; break;

		case 'A': val = 0b01110111; break;
		case 'a': val = 0b01110111; break;
		case 'B': val = 0b00011111; break;
		case 'b': val = 0b00011111; break;
		case 'C': val = 0b01001110; break;
		case 'c': val = 0b01001110; break;
		case 'D': val = 0b00111101; break;
		case 'd': val = 0b00111101; break;
		case 'E': val = 0b01001111; break;
		case 'F': val = 0b01000111; break;
		case 'G': val = 0b01011110; break;
		case 'H': val = 0b00110111; break;
		case 'h': val = 0b00010111; break;
		case 'J': val = 0b00111100; break;
		case 'L': val = 0b00001110; break;
		case 'O': val = 0b01111110; break;
		case 'P': val = 0b01100111; break;
		case 'r': val = 0b00000101; break;
		case 'S': val = 0b01011011; break;
		case 't': val = 0b00001111; break;
		case 'U': val = 0b00111110; break;
		case 'Y': val = 0b00111110; break;
		case '-': val = 0b00000001; break;
		case '_': val = 0b00001000; break;
		case '=': val = 0b00001001; break;

		default: val = 0; break;
	}
	return val;
}

/** @brief Sends numeric data to a LED indicator
 
  If area size is not enough for negative sign
  function will display "o" char instead leftest 
  digit

 @param [in] max7219* led Control object for MAX7219 chips
 @param [in] int32_t numeric Value for display
 @param [in] uint8_t start_index Start position for displaying. Starts from 0
 @param [in] uint8_t area_size Area size for displaying
 @param [in] uint8_t zerolead_flag Show leading zeros or not
 @param [in] uint8_t thousanddot_flag Show dot as separator for thousands or not
 @param [in] uint8_t instantUpdate Display on a LED immidiately or not
 */
void max7219_send_numeric(max7219* led, int32_t numeric, uint8_t start_index, uint8_t area_size, uint8_t zerolead_flag, uint8_t thousanddot_flag, uint8_t instantUpdate){
	uint8_t negative = 0;
	uint8_t pStart = 0;
	uint8_t pEnd = 0;
	if (numeric < 0) { negative = 1; }

	if ((led->ctrlNumber *8) <  start_index + area_size){
		area_size = ((led->ctrlNumber *8) - start_index);
	}
	pStart = start_index;
	pEnd = start_index+area_size;

	for(uint8_t p = pStart; p < pEnd; p++){
		if (!zerolead_flag) {
			if (CHECK_CODE_B_BIT(led->ledOptions[p])){
				led->ledData[p] = MAX7219_CHAR_BLANK;
			} else {
				led->ledData[p] = 0;	
			}
		} else {
			if (CHECK_CODE_B_BIT(led->ledOptions[p])){
				led->ledData[p] = 0;
			} else {
				led->ledData[p] = 0b01111110;	
			}	
		}
	}

	for(uint8_t p = pStart; p < pEnd; p++){
		uint8_t dig, signChar, overError;
		if (CHECK_CODE_B_BIT(led->ledOptions[p])){
			dig = numeric % 10;
			signChar = MAX7219_CHAR_NEGATIVE;
			overError = MAX7219_CHAR_E;
		} else {
			dig = max7219_num2bitmap(numeric % 10);
			signChar = 0b00000001;
			overError = 0b01001111;
		}
		if ((thousanddot_flag) && (((p+1) % 4) == 0)){
			dig |= 1 << 7;
		}
		led->ledData[p] = dig;
		numeric /= 10;
		if (numeric == 0){
			if (negative){
				if (p+1 < pEnd){
					led->ledData[p+1] = signChar;
				} else {
					led->ledData[p] = overError;
				}
			}
			break;
		}
	}
	if (instantUpdate){
		max7219_display_data(led);
	}
}

/** @brief Sends ASCII data to a LED indicator
 
  This function can't display text on the places
  with Code B bit

 @param [in] max7219* led Control object for MAX7219 chips
 @param [in] char* text ASCII text for display
 @param [in] uint8_t start_index Start position for displaying. Starts from 0
 @param [in] uint8_t area_size Area size for displaying
 @param [in] uint8_t instantUpdate Display on a LED immidiately or not
 */
void max7219_send_text(max7219* led, char* text, uint8_t start_index, uint8_t area_size, uint8_t instantUpdate){
	uint8_t strLen = strlen(text);
	uint8_t dotCount = 0;
	for (uint8_t c = 0; c < strLen; c++){
		if ((text[c] == ',') || (text[c] == '.')){
			dotCount++;
		}
	}
	if ((strLen - dotCount) > (area_size)){
		strLen = area_size;
	}
	uint8_t charPointer = strLen;
	uint8_t nextDotFlag = 0;
	uint8_t ledCharPointer= start_index;
	while(charPointer > 0){
		if ((text[charPointer-1] == '.') || (text[charPointer-1] == ',')){
			nextDotFlag = 1;
		} else {
			uint8_t bitmap = max7219_char2bitmap(text[charPointer-1]);
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
		max7219_display_data(led);
	}
}
