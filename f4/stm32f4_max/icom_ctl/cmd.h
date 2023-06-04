#include <string.h>
#include <stdint.h>

#define CMD_FSM_WAITING 0
#define CMD_FSM_END 1
#define CMD_FSM_FREQ_SET_START 2
#define CMD_FSM_RCVR_PARAM_SET_START 3

#define REPLY_FSM_WAITING 0 
#define REPLY_FSM_SIGNAL_PARAMS 1

#define FREQ_SETUP_STR_LENGHT  18
#define RCVR_PARAM_SETUP_STR_LENGHT  5

#define SIGNAL_PARAM_REPLY_STR_LENGTH 3

typedef struct {
	char freq[10];
	char modulation[3];
	char filter[4];
} FreqParams;

typedef struct {
	uint8_t volume;
	char strVolume[2];
	uint8_t squelch_level;
	char strSquelch[3];
	uint8_t if_shift_value;
	uint8_t agc_state;
	char strAgc[4];
	uint8_t noise_blanker_state;
	char strNb[3];
	uint8_t attenuator_state;
	char strAtt[4];
	uint8_t signalLevel;
	uint8_t sqlState;
	uint8_t sigCenter;
} RecieverParams;

void detect_start_cmd(char s);
void detect_start_reply(char s);
uint8_t strHex2int(char c);
uint8_t get_int_rcvr_param(char* strValue);
void value2symbol(uint8_t value, char* s);
void parse_vol_level(void);
void parse_sq_level(void);
void parse_if_shift(void);
void parse_agc_state(void);
void parse_nb_state(void);
void parse_att_state(void);
void parse_rcvr_param_string(void);
void parse_freq_string(void);
void handle_end_cmd(void);
void read_freq_setup(char s);
void read_rcvr_param_setup(char s);
void parse_signal_param(void);
void read_signal_param_reply(char s);
void host_cmd_parser(uint8_t in_byte);
void radio_reply_parser(uint8_t out_byte);

extern FreqParams char_freq_params;
extern RecieverParams int_rcvr_params;
