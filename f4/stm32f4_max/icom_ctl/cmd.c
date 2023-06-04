#include "cmd.h"

#include <libopencm3/stm32/gpio.h>
uint8_t cmd_parser_fsm = CMD_FSM_WAITING;
uint8_t reply_parser_fsm = REPLY_FSM_WAITING;
char freq_setup[FREQ_SETUP_STR_LENGHT];
uint8_t freq_char_index = 0;


FreqParams char_freq_params = {"0000000000", " CW", "2.8k"};
RecieverParams int_rcvr_params = {0,"\x07\x20", 0,"Sq\x20", 0, 0, "Ag0 ", 0, "Nb0", 0, " At0", 0, 0, 0};

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
	int_rcvr_params.strAgc[3] =  int_rcvr_params.agc_state ? '1' : '0';
	cmd_parser_fsm = CMD_FSM_WAITING;
}
void parse_nb_state(void){
	int_rcvr_params.noise_blanker_state = get_int_rcvr_param(&rcvr_param_setup[2]);
	int_rcvr_params.strNb[2] =  int_rcvr_params.noise_blanker_state ? '1' : '0';
	cmd_parser_fsm = CMD_FSM_WAITING;
}
void parse_att_state(void){
	int_rcvr_params.attenuator_state = get_int_rcvr_param(&rcvr_param_setup[2]);
	int_rcvr_params.strAtt[3] =  int_rcvr_params.attenuator_state ? '1' : '0';
	cmd_parser_fsm = CMD_FSM_WAITING;
}

void parse_rcvr_param_string(void){
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

void parse_freq_string(void){
	for(uint8_t f = 0; f < 10; f++){
		char_freq_params.freq[f] = freq_setup[f+1];
	}
	uint8_t mod_id = (uint8_t)freq_setup[12] - (uint8_t)freq_setup[11];
	uint8_t band_id = (uint8_t)freq_setup[14] - (uint8_t)freq_setup[13];
	switch (mod_id){
		case 0: memcpy(char_freq_params.modulation, "LSB", 3); break;
		case 1: memcpy(char_freq_params.modulation, "USB", 3); break;
		case 2: memcpy(char_freq_params.modulation, " AM", 3); break;
		case 3: memcpy(char_freq_params.modulation, " CW", 3); break;
		case 4: memcpy(char_freq_params.modulation, "---", 3); break;
		case 5: memcpy(char_freq_params.modulation, "NFM", 3); break;
		case 6: memcpy(char_freq_params.modulation, "WFM", 3); break;
		default: break;
	}
	switch (band_id){
		case 0: memcpy(char_freq_params.filter, "2.8k", 4); break;
		case 1: memcpy(char_freq_params.filter, "  6k", 4); break;
		case 2: memcpy(char_freq_params.filter, " 15k", 4); break;
		case 3: memcpy(char_freq_params.filter, " 50k", 4); break;
		case 4: memcpy(char_freq_params.filter, "230k", 4); break;
		default: break;
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
		case  CMD_FSM_FREQ_SET_START: parse_freq_string();  break;
		case CMD_FSM_RCVR_PARAM_SET_START: parse_rcvr_param_string();  break;
		default: break;
	}
}

void read_freq_setup(char s){
	if (s != '\n'){
		freq_setup[freq_char_index] = s;
		if(freq_char_index + 1 >= FREQ_SETUP_STR_LENGHT){
			freq_char_index = 0;
		} else {
			freq_char_index++;
		}
	} else {
		handle_end_cmd();
	}
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
        gpio_toggle(GPIOE, GPIO8);
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
