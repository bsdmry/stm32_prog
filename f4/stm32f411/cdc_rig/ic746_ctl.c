#include "ic746_ctl.h"
#include "usart.h"
#include <inttypes.h>
#include <stdio.h>

uint8_t rigcmd[CAT_CMD_BUF_LENGTH] = {0};

static void(*catSplit)(uint8_t);
static void(*catSetPtt)(uint8_t);
static uint8_t(*catGetPtt)(void);
static long(*catGetFreq)(void);
static void(*catSetFreq)(long);
static uint8_t(*catGetMode)(void);
static void(*catSetMode)(uint8_t);
static uint8_t(*catGetSmeter)(void);
static void(*catSetVFO)(uint8_t);
static void(*catAtoB)(void);
static void(*catSwapVfo)(void);

Rig746 ic746_rig = {.cmdbuffer = rigcmd, .fsm_state = CAT_RCV_WAITING, .cmdlen = 0, .maxlen = CAT_CMD_BUF_LENGTH};

uint8_t ic746_get_command(void){
	uint8_t bt;
	uint8_t cmdRcvd = 0;
	uint8_t length = 0;
	while ((rx_ring_buffer.length != 0) && !cmdRcvd){
		buf_pop_u8(&rx_ring_buffer, &bt);
		//
		char s[12];
		sprintf(s, "BYTE 0x%02"PRIx8"\r\n", bt);
		usart_send_string(USART2, s, 12);
		//
		switch(ic746_rig.fsm_state){
			case CAT_RCV_WAITING:
				usart_send_string(USART2, "FSM_FIST_PRE\n", 14);
				if (bt == CAT_PREAMBLE) { ic746_rig.fsm_state = CAT_RCV_INIT; } break;
			case CAT_RCV_INIT:
				usart_send_string(USART2, "FSM_SECO_PRE\n", 14);
				if (bt == CAT_PREAMBLE) {
					ic746_rig.fsm_state = CAT_RCV_RECEIVING;
				} else {
					ic746_rig.fsm_state = CAT_RCV_WAITING;
					ic746_send_nack();
				}
				break;
			case CAT_RCV_RECEIVING:
				switch (bt){
					case CAT_EOM:
						usart_send_string(USART2, "FSM_RECV_END\n", 14);
						ic746_rig.fsm_state = CAT_RCV_WAITING;
						cmdRcvd = 1;
						ic746_rig.cmdlen = length;
						length = 0;
						ic746_send_echo();
						break;
					default:
						usart_send_string(USART2, "FSM_RCV_DATE\n", 14);
						if (length <= ic746_rig.maxlen){
							ic746_rig.cmdbuffer[length] = bt;
							length++;
						} else {
							ic746_rig.fsm_state = CAT_RCV_WAITING;
							length = 0;
							ic746_clean_command_buffer();
							ic746_send_nack();
						}
						break;
				}
				break;
		}
	//
	char sa[22];
	sprintf(sa, "RXB L: %02"PRIu8" cmdRcv: %02"PRIu8"\n", rx_ring_buffer.length, cmdRcvd);
	usart_send_string(USART2, sa, 22);
	//
	}
	return cmdRcvd;
}

void ic746_parse_command(void){
	usart_send_string(USART2, "Start parse CMD: ", 18);
	for (uint8_t i = 0; i < ic746_rig.cmdlen; i++){
		char s[7];
		sprintf(s, "<0x%02"PRIx8" ", ic746_rig.cmdbuffer[i]);
		usart_send_string(USART2, s, 6);
	}
	usart_send_string(USART2, "\r\n", 3);

	switch (ic746_rig.cmdbuffer[CAT_IX_CMD]){
		case CAT_PTT: doPtt(); break;
		case CAT_SPLIT: doSplit(); break;
		case CAT_SET_VFO: doSetVfo(); break;
		case CAT_SET_FREQ: doSetFreq(); break;
		case CAT_SET_MODE: doSetMode(); break;
		case CAT_READ_MODE: doReadMode(); break;
		case CAT_READ_FREQ: doReadFreq(); break;
		case CAT_READ_SMETER: doSmeter(); break;
		case CAT_MISC: doMisc(); break;
		case CAT_READ_ID: doReadId(); break;
		case CAT_SET_RD_STEP: doTuneStep(); break;
		case CAT_SET_RD_ANT: doAntSel(); break;
		case CAT_SET_RD_ATT:
		case CAT_SET_RD_PARAMS2: doUnimplemented_1b(); break;
		case CAT_SET_RD_PARAMS1:
		case CAT_READ_OFFSET: doUnimplemented_2b(); break;
		default: usart_send_string(USART2, "Unknow command\r\n", 17); ic746_send_nack(); break;
	}
}

void ic746_clean_command_buffer(void){
	for (uint8_t i = 0; i < ic746_rig.maxlen; i++){
		ic746_rig.cmdbuffer[i] = 0;
	}
	ic746_rig.cmdlen = 0;

}

void ic746_send_echo(void){
	uint8_t * resp = malloc(ic746_rig.cmdlen+3);
	//uint8_t resp[ic746_rig.cmdlen+3] = {0};
	resp[0] = CAT_PREAMBLE;
	resp[1] = CAT_PREAMBLE;
	for (uint8_t i = 0; i < ic746_rig.cmdlen; i++ ){
		resp[i+2] = ic746_rig.cmdbuffer[i];
	}
	resp[ic746_rig.cmdlen+2] = CAT_EOM;
	//
	usart_send_string(USART2, "SEND ECHO: ", 12);
	for (uint8_t i = 0; i < ic746_rig.cmdlen+3; i++){
		char s[7];
		sprintf(s, ">0x%02"PRIx8" ", resp[i]);
		usart_send_string(USART2, s, 7);
	}
	usart_send_string(USART2, "\r\n", 3);
	//
	cdc_send(resp, ic746_rig.cmdlen+3);
	free(resp);
}

void ic746_send_response(uint8_t cmdlen){
	uint8_t * resp = malloc(cmdlen+3);
	//uint8_t resp[cmdlen+3] = {0};
	resp[0] = CAT_PREAMBLE;
	resp[1] = CAT_PREAMBLE;
	resp[2] = ic746_rig.cmdbuffer[CAT_IX_FROM_ADDR];
	resp[3] = ic746_rig.cmdbuffer[CAT_IX_TO_ADDR];
	for (uint8_t i = CAT_IX_CMD; i < cmdlen; i++ ){
		resp[i+2] = ic746_rig.cmdbuffer[i];
	}
	resp[cmdlen+2] = CAT_EOM;
	//
	usart_send_string(USART2, "SEND RESP: ", 12);
	for (uint8_t i = 0; i < cmdlen+3; i++){
		char s[7];
		sprintf(s, ">0x%02"PRIx8" ", resp[i]);
		usart_send_string(USART2, s, 7);
	}
	usart_send_string(USART2, "\r\n", 3);
	//

	cdc_send(resp, cmdlen+3);
	free(resp);
}



void ic746_send_nack(void){
	usart_send_string(USART2, "SEND NACK\n", 11);
	uint8_t nack[6] = {CAT_PREAMBLE, CAT_PREAMBLE, CAT_CTRL_ADDR, CAT_RIG_ADDR, CAT_NACK, CAT_EOM};
	ic746_clean_command_buffer();
	cdc_send(nack, 6);
}

void ic746_send_ack(void){
	usart_send_string(USART2, "SEND ACK\n", 10);
	uint8_t ack[6] = {CAT_PREAMBLE, CAT_PREAMBLE, CAT_CTRL_ADDR, CAT_RIG_ADDR, CAT_ACK, CAT_EOM};
	ic746_clean_command_buffer();
	cdc_send(ack, 6);
}

void ic746_add_handler_set_ptt(void (*userFunc)(uint8_t)) { catSetPtt = userFunc; }
void ic746_add_handler_get_ptt(uint8_t (*userFunc)(void)) { catGetPtt = userFunc; }
void ic746_add_handler_get_freq(long (*userFunc)(void)) { catGetFreq = userFunc; }
void ic746_add_handler_set_freq(void (*userFunc)(long)) { catSetFreq = userFunc; }
void ic746_add_handler_get_mode(uint8_t (*userFunc)(void)) { catGetMode = userFunc; }
void ic746_add_handler_set_mode(void (*userFunc)(uint8_t)) { catSetMode = userFunc; }
void ic746_add_handler_split(void (*userFunc)(uint8_t)) { catSplit = userFunc; }
void ic746_add_handler_AtoB(void (*userFunc)(void)) { catAtoB = userFunc; }
void ic746_add_handler_swap_vfo(void (*userFunc)(void)) { catSwapVfo = userFunc; }
void ic746_add_handler_get_smeter(uint8_t(*userFunc)(void)) { catGetSmeter = userFunc; }
void ic746_add_handler_set_vfo(void (*userFunc)(uint8_t)) { catSetVFO = userFunc; }



void doPtt(void){
  if (ic746_rig.cmdlen == CAT_RD_LEN_SUB) {  // Read request
    if (catGetPtt) {
      ic746_rig.cmdbuffer[CAT_IX_PTT] = catGetPtt();
      ic746_send_response(CAT_SZ_PTT);
    }
  } else {               // Set request
    if (catSetPtt) {
      if (ic746_rig.cmdbuffer[CAT_IX_PTT] == CAT_PTT_TX) { catSetPtt(1); } else { catSetPtt(0); }
    }
    ic746_send_ack();  // always acknowledge "set" commands
  }
}

void doSmeter(void) {
  switch (ic746_rig.cmdbuffer[CAT_IX_SUB_CMD]) {
    case CAT_READ_SUB_SMETER:
      if (catGetSmeter) {
                          //S0  S1  S2  S3  S4  S5  S6  S7   S8   S9  +10  +20  +30  +40  +50  +60
        const uint8_t smap[] = {0, 15, 25, 40, 55, 65, 75, 90, 100, 120, 135, 150, 170, 190, 210, 241};
        uint8_t s = catGetSmeter();

        SmetertoBCD(smap[s]);
      } else {
        ic746_rig.cmdbuffer[CAT_IX_SMETER] = 0;      // user has not supplied S Meter function - keep the protocol happy
        ic746_rig.cmdbuffer[CAT_IX_SMETER + 1] = 0;
      }
      ic746_send_response(CAT_SZ_SMETER);
      break;

    case CAT_READ_SUB_SQL:        // Squelch condition 0=closed, 1=open
      ic746_rig.cmdbuffer[CAT_IX_SQUELCH] = 1;
      ic746_send_response(CAT_SZ_SQUELCH);
      break;
  }
}


void doSplit(void) {
  switch (ic746_rig.cmdbuffer[CAT_IX_SUB_CMD]) {
    case CAT_SPLIT_OFF: if (catSplit) { catSplit(0); } break;
    case CAT_SPLIT_ON:
    case CAT_SIMPLE_DUP: if (catSplit) { catSplit(1); }
    default: break;
  }
  ic746_send_ack(); 
}

void doSetVfo(void) {

  if (ic746_rig.cmdlen == CAT_RD_LEN_NOSUB) {  // No sub-command - sets VFO Tuning vice memory tuning
    ic746_send_ack(); // Memory tuning is not implemented so send ack to keep protocol happy
    return;
  }

  switch (ic746_rig.cmdbuffer[CAT_IX_SUB_CMD]) {
    case CAT_VFO_A:
    case CAT_VFO_B:
      if (catSetVFO) { catSetVFO(ic746_rig.cmdbuffer[CAT_IX_SUB_CMD]); } break;
    case CAT_VFO_A_TO_B:
      if (catAtoB) { catAtoB(); } break;
    case CAT_VFO_SWAP:
      if (catSwapVfo) { catSwapVfo(); } break;
  }
  ic746_send_ack(); 
}

void doSetFreq(void) {
  if (catSetFreq) { catSetFreq(BCDtoFreq()); } // Convert the frequency BCD to Long and call the user function
  ic746_send_ack(); 
}

void doReadFreq(void) {
  usart_send_string(USART2, "CMD: READ_FREQ\r\n", 17);
  if (catGetFreq) {
    FreqtoBCD(catGetFreq());  // get the frequency, convert to BCD and stuff it in the response buffer
    ic746_send_response(CAT_SZ_FREQ);
  }
}

void doSetMode(void) {
  if (catSetMode) {
    switch (ic746_rig.cmdbuffer[CAT_IX_SUB_CMD]) {
      case CAT_MODE_LSB:
      case CAT_MODE_USB: catSetMode(ic746_rig.cmdbuffer[CAT_IX_SUB_CMD]); break;
    }
  }
  ic746_send_ack(); 
}

void doReadMode(void) {
  if (catGetMode) {
    ic746_rig.cmdbuffer[CAT_IX_MODE] = catGetMode();
    ic746_rig.cmdbuffer[CAT_IX_MODE+1] = CAT_MODE_FILTER1;  // protocol filter - return reasonable value
    ic746_send_response(CAT_SZ_MODE);
  }
}


void doReadId(void){
    ic746_rig.cmdbuffer[CAT_SZ_ID - 1] = 56;
    ic746_send_response(CAT_SZ_ID);
}

void doMisc(void) {
  switch (ic746_rig.cmdbuffer[CAT_IX_SUB_CMD]) {
    case CAT_READ_IF_FILTER:
      ic746_rig.cmdbuffer[CAT_IX_IF_FILTER] = 0;
      ic746_send_response(CAT_SZ_IF_FILTER);
      break;

    // Not implemented
    // Reply with ACK to keep the protocol happy
    case CAT_SET_MEM_CHAN:
    case CAT_SET_BANDSTACK:
    case CAT_SET_MEM_KEYER:
      ic746_send_ack(); 
      break;
  }
}

void doUnimplemented_1b(void) {
  if (ic746_rig.cmdlen == CAT_RD_LEN_SUB) {        // Read request
    ic746_rig.cmdbuffer[CAT_IX_DATA] = 0;   // return 0 for all read requests
    ic746_send_response(CAT_SZ_UNIMP_1B);
  } else {                   // Set parameter request
    ic746_send_ack();               // Send an acknowledgement to keep the protocol happy
  }
}

void doUnimplemented_2b(void) {
  if (ic746_rig.cmdlen == CAT_RD_LEN_SUB) {        // Read request
    ic746_rig.cmdbuffer[CAT_IX_DATA] = 0;   // return 0 for all read requests
    ic746_rig.cmdbuffer[CAT_IX_DATA+1] = 0; 
    ic746_send_response(CAT_SZ_UNIMP_2B);
  } else {                   // Set parameter request
    ic746_send_ack();               // Send an acknowledgement to keep the protocol happy
  }
}

void doTuneStep(void) {
  if (ic746_rig.cmdlen == CAT_RD_LEN_NOSUB) {             // Read request
    ic746_rig.cmdbuffer[CAT_IX_TUNE_STEP] = 0;   // return 0 for all read requests
    ic746_send_response(CAT_SZ_TUNE_STEP);
  } else {                   // Set parameter request
    ic746_send_ack();               // Send an acknowledgement to keep the protocol happy
  }
}

void doAntSel(void) {
  if (ic746_rig.cmdlen == CAT_RD_LEN_NOSUB) {           // Read request
    ic746_rig.cmdbuffer[CAT_IX_ANT_SEL] = 0;   // return 0 for all read requests
    ic746_send_response(CAT_SZ_ANT_SEL);
  } else {                   // Set parameter request
    ic746_send_ack();               // Send an acknowledgement to keep the protocol happy
  }
}

long BCDtoFreq(void) {
  long freq;

  freq = ic746_rig.cmdbuffer[CAT_IX_FREQ] & 0xf;                 // lower 4 bits
  freq += 10L * (ic746_rig.cmdbuffer[CAT_IX_FREQ] >> 4);          // upper 4 bits
  freq += 100L * (ic746_rig.cmdbuffer[CAT_IX_FREQ + 1] & 0xf);
  freq += 1000L * (ic746_rig.cmdbuffer[CAT_IX_FREQ + 1] >> 4);
  freq += 10000L * (ic746_rig.cmdbuffer[CAT_IX_FREQ + 2] & 0xf);
  freq += 100000L * (ic746_rig.cmdbuffer[CAT_IX_FREQ + 2] >> 4);
  freq += 1000000L * (ic746_rig.cmdbuffer[CAT_IX_FREQ + 3] & 0xf);
  freq += 10000000L * (ic746_rig.cmdbuffer[CAT_IX_FREQ + 3] >> 4);

  return freq;
}

void FreqtoBCD(long freq) {
  uint8_t ones, tens, hund, thou, ten_thou, hund_thou, mil, ten_mil;

  ones =     (uint8_t)(freq % 10);
  tens =     (uint8_t)((freq / 10L) % 10);
  ic746_rig.cmdbuffer[CAT_IX_FREQ] = (uint8_t)((tens << 4)) | ones;

  hund =      (uint8_t)((freq / 100L) % 10);
  thou =      (uint8_t)((freq / 1000L) % 10);
  ic746_rig.cmdbuffer[CAT_IX_FREQ + 1] = (uint8_t)((thou << 4)) | hund;

  ten_thou =  (uint8_t)((freq / 10000L) % 10);
  hund_thou = (uint8_t)((freq / 100000L) % 10);
  ic746_rig.cmdbuffer[CAT_IX_FREQ + 2] = (uint8_t)((hund_thou << 4)) | ten_thou;

  mil =       (uint8_t)((freq / 1000000L) % 10);
  ten_mil =   (uint8_t)(freq / 10000000L);
  ic746_rig.cmdbuffer[CAT_IX_FREQ + 3] = (uint8_t)((ten_mil << 4)) | mil;
  ic746_rig.cmdbuffer[CAT_IX_FREQ + 4] = 0; // fixed

}

void SmetertoBCD(uint8_t s) {
  uint8_t ones, tens, hund;

  ones =     (uint8_t)(s % 10);
  tens =     (uint8_t)((s / (uint8_t)(10)) % 10);
  ic746_rig.cmdbuffer[CAT_IX_SMETER+1] = (uint8_t)((tens << 4)) | ones;

  hund =      (uint8_t)((s / (uint8_t)(100)) % 10);
  ic746_rig.cmdbuffer[CAT_IX_SMETER] = hund;

}
