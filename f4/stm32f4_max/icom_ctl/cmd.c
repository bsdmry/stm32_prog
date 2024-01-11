#include "cmd.h"
#include "usart.h"
#include <libopencm3/stm32/gpio.h>
uint8_t cmd_parser_fsm = CMD_FSM_WAITING;
uint8_t reply_parser_fsm = REPLY_FSM_WAITING;
char freq_setup[FREQ_SETUP_STR_LENGHT];
uint8_t freq_char_index = 0;


FreqParams char_freq_params = {
	.freq = "0001000000", 
	.modulation =  " CW", 
	.filter =  "2.8k"
};
RecieverParams int_rcvr_params = {
	.volume = 128, 
	.strVolume = "\x07\x20", 
	.squelch_level = 0, 
	.strSquelch = "Sq\x20", 
	.if_shift_value = 0, 
	.agc_state = 0, 
	.noise_blanker_state = 0, 
	.attenuator_state =  0, 
	.signalLevel =  0, 
	.sqlState = 0, 
	.sigCenter =  0,
	.wasInit = ICOM_HASNT_CFG,
	.controlMode = CONTROL_MODE_NONE,
	.frequency = 1000000,
	.modulation = 3,
	.filter = 0,
	.step = 1000
};

char rcvr_param_setup[RCVR_PARAM_SETUP_STR_LENGHT];
uint8_t rcvr_param_char_index = 0;

char signal_param_reply[SIGNAL_PARAM_REPLY_STR_LENGTH];
uint8_t sig_param_char_index = 0;

uint8_t strHex2int(char c){
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	return 0;
}

void int2str(uint32_t val, uint8_t strLen, char* recv){
        uint8_t dig;
        for (uint8_t p = 0; p < strLen; p++) {
                dig = val % 10;
                recv[strLen - p - 1] = (char)(dig + 48);
                val /= 10;
        }
}

void int2strhex(uint32_t val, uint8_t strLen, char* recv){
        uint8_t dig;
        for (uint8_t p = 0; p < strLen; p++) {
                dig = val % 0x10;
                if (dig > 9){
                        recv[strLen - p - 1] = (char)(dig + 55);
                } else {
                        recv[strLen - p - 1] = (char)(dig + 48);
                }
                val /= 0x10;
        }
}

uint8_t get_int_rcvr_param(char* strValue){
	uint8_t n1 = strHex2int(strValue[0]);
	uint8_t n2 = strHex2int(strValue[1]);
	return ((n1 << 4) ^  n2);
}


void parse_vol_level(void){
	int_rcvr_params.volume = get_int_rcvr_param(&rcvr_param_setup[2]);
	cmd_parser_fsm = CMD_FSM_WAITING;
}
void parse_sq_level(void){
	int_rcvr_params.squelch_level = get_int_rcvr_param(&rcvr_param_setup[2]);
	cmd_parser_fsm = CMD_FSM_WAITING;
}
void parse_if_shift(void){
	int_rcvr_params.if_shift_value = get_int_rcvr_param(&rcvr_param_setup[2]);
	cmd_parser_fsm = CMD_FSM_WAITING;
}
void parse_agc_state(void){
	int_rcvr_params.agc_state = get_int_rcvr_param(&rcvr_param_setup[2]);
	cmd_parser_fsm = CMD_FSM_WAITING;
}
void parse_nb_state(void){
	int_rcvr_params.noise_blanker_state = get_int_rcvr_param(&rcvr_param_setup[2]);
	cmd_parser_fsm = CMD_FSM_WAITING;
}
void parse_att_state(void){
	int_rcvr_params.attenuator_state = get_int_rcvr_param(&rcvr_param_setup[2]);
	cmd_parser_fsm = CMD_FSM_WAITING;
}

void parse_rcvr_param_string(void){
	if (int_rcvr_params.controlMode == CONTROL_MODE_BRIDGE){
		switch( ((uint8_t)rcvr_param_setup[0] ^ (uint8_t)rcvr_param_setup[1])  ){
			case 0x04: parse_vol_level(); break;
			case 0x05: parse_sq_level(); break;
			case 0x07: parse_if_shift(); break;
			case 0x01: parse_agc_state(); break;
			case 0x02: parse_nb_state(); break;
			case 0x03: parse_att_state(); break;
			default: break;
		}
	}
}

void parse_freq_string(uint8_t cmdLen){
        gpio_toggle(GPIOE, GPIO8);
	usart_send_string(USART2, freq_setup, FREQ_SETUP_STR_LENGHT);
	usart_send_string(USART2, "=\n", 2);
	for(uint8_t f = 0; f < 10; f++){
		char_freq_params.freq[f] = freq_setup[f+1];
	}
	uint8_t mod_id = (uint8_t)freq_setup[12] - (uint8_t)freq_setup[11];
	switch (mod_id){
		case 0: memcpy(char_freq_params.modulation, "LSB", 3); break;
		case 1: memcpy(char_freq_params.modulation, "USB", 3); break;
		case 2: memcpy(char_freq_params.modulation, " AM", 3); break;
		case 3: memcpy(char_freq_params.modulation, " CW", 3); break;
		case 4: memcpy(char_freq_params.modulation, "---", 3); break;
		case 5: memcpy(char_freq_params.modulation, "NFM", 3); break;
		case 6: memcpy(char_freq_params.modulation, "WFM", 3); memcpy(char_freq_params.filter, "230k", 4); break;
		default: break;
	}
	uint8_t band_id = (uint8_t)freq_setup[14] - (uint8_t)freq_setup[13];
	switch (band_id){
		case 0: memcpy(char_freq_params.filter, "2.8k", 4); break;
		case 1: memcpy(char_freq_params.filter, "  6k", 4); break;
		case 2: memcpy(char_freq_params.filter, " 15k", 4); break;
		case 3: memcpy(char_freq_params.filter, " 50k", 4); break;
		case 4: memcpy(char_freq_params.filter, "230k", 4); break;
		default: break;
	}
	if (int_rcvr_params.controlMode == CONTROL_MODE_BRIDGE){
		int_rcvr_params.frequency = (uint32_t)strtol(char_freq_params.freq, NULL, 10);
		int_rcvr_params.modulation = mod_id;
		int_rcvr_params.filter = band_id;
	}
	cmd_parser_fsm = CMD_FSM_WAITING;
};

void detect_start_cmd(char s){
	switch (s){	
		case 'K': cmd_parser_fsm = CMD_FSM_FREQ_SET_START; break;
		case 'J': cmd_parser_fsm = CMD_FSM_RCVR_PARAM_SET_START; break;
		default: break;
	}
}

void detect_start_reply(char s){
	switch(s){
		case 'I': reply_parser_fsm = REPLY_FSM_SIGNAL_PARAMS; break;
		default: break;
	}
}

void handle_end_cmd(void){
	switch (cmd_parser_fsm){	
		case  CMD_FSM_FREQ_SET_START: 
			parse_freq_string(freq_char_index);  
			freq_char_index = 0;
			break;
		case CMD_FSM_RCVR_PARAM_SET_START: parse_rcvr_param_string();  break;
		default: break;
	}
}

void read_freq_setup(char s){
	
	if (((uint8_t)s >= 48) && ((uint8_t)s <= 57)){
		freq_setup[freq_char_index] = s;
		freq_char_index++;
	} else if (s == '\r'){
		freq_setup[freq_char_index] = s;
		freq_char_index++;
	} else if (s == '\n'){	
		handle_end_cmd();
	} else {
		freq_char_index = 0;
		cmd_parser_fsm = CMD_FSM_WAITING;
	}

	/*if (s != '\n'){
		freq_setup[freq_char_index] = s;
		if(freq_char_index + 1 >= FREQ_SETUP_STR_LENGHT){
			freq_char_index = 0;
		} else {
			freq_char_index++;
		}
	} else {
		handle_end_cmd();
	}*/
}

void read_rcvr_param_setup(char s){
	if (s != '\n'){
		rcvr_param_setup[rcvr_param_char_index] = s;
		if (rcvr_param_char_index +1 >= RCVR_PARAM_SETUP_STR_LENGHT){
			rcvr_param_char_index = 0;
		} else {
			rcvr_param_char_index++;
		}

	} else {
		handle_end_cmd();
	}
}


void parse_signal_param(void){
	switch (signal_param_reply[0]){
		case '0': int_rcvr_params.sqlState = get_int_rcvr_param(&signal_param_reply[1]); break;
		case '1': int_rcvr_params.signalLevel = get_int_rcvr_param(&signal_param_reply[1]); break;
		case '2': int_rcvr_params.sigCenter = get_int_rcvr_param(&signal_param_reply[1]); break;
		default: break;

	}
}

void read_signal_param_reply(char s){
	signal_param_reply[sig_param_char_index] = s;
	if ((sig_param_char_index+1) >= SIGNAL_PARAM_REPLY_STR_LENGTH){
		parse_signal_param();
		sig_param_char_index = 0;
		reply_parser_fsm = REPLY_FSM_WAITING;
	} else {
		sig_param_char_index++;
	}
}

void host_cmd_parser(uint8_t in_byte){
	switch (cmd_parser_fsm){	
		case CMD_FSM_WAITING: detect_start_cmd((char)in_byte); break;
		case CMD_FSM_FREQ_SET_START: read_freq_setup((char)in_byte); break;
		case CMD_FSM_RCVR_PARAM_SET_START: read_rcvr_param_setup((char)in_byte); break;
		default: break;
	}
}

void radio_reply_parser(uint8_t out_byte){
	switch(reply_parser_fsm){
		case REPLY_FSM_WAITING: detect_start_reply((char)out_byte); break;
		case REPLY_FSM_SIGNAL_PARAMS: read_signal_param_reply((char)out_byte); break;
		default: break;
	}
}
