#ifndef IC746_CTL_H
#define IC746_CTL_H

#include "cbuf.h"
#include "usb_cdc.h"
#define CAT_VER "1.1"
/*
   CAT Command definitions from IC746 Manual
*/

// Protocol
#define CAT_PREAMBLE        0xFE  // sent twice at start of command
#define CAT_EOM             0xFD  // end of message
#define CAT_ACK             0xFB  // OK
#define CAT_NACK            0xFA  // No good
#define CAT_RIG_ADDR        0x56  // Rig ID for IC746
#define CAT_CTRL_ADDR       0xE0  // Controller ID

// Commands
#define CAT_SET_TCV_FREQ    0x00  // Not implemented
#define CAT_SET_TCV_MODE    0x01  // Not implemented
#define CAT_READ_BAND_EDGE  0x02  // Not implemented
#define CAT_READ_FREQ       0x03
#define CAT_READ_MODE       0x04
#define CAT_SET_FREQ        0x05
#define CAT_SET_MODE        0x06
#define CAT_SET_VFO         0x07
#define CAT_SEL_MEM         0x08  // Not implemented
#define CAT_WRITE_MEM       0x09  // Not implemented
#define CAT_MEM_TO_VFO      0x0A  // Not implemented
#define CAT_CLEAR_MEM       0x0B  // Not implemented
#define CAT_READ_OFFSET     0x0C  // Not implemented
#define CAT_SET_OFFSET      0x0D  // Not implemented
#define CAT_SCAN            0x0E  // Not implemented
#define CAT_SPLIT           0x0F
#define CAT_SET_RD_STEP     0x10  // Not implemented
#define CAT_SET_RD_ATT      0x11  // Not implemented
#define CAT_SET_RD_ANT      0x12  // Not implemented
#define CAT_SET_UT102       0x13  // Not implemented
#define CAT_SET_RD_PARAMS1  0x14  // Not implemented
#define CAT_READ_SMETER     0x15  // Only impemented read S-Meter
#define CAT_SET_RD_PARAMS2  0x16  // Not implemented (various settings)
#define CAT_READ_ID         0x19  
#define CAT_MISC            0x1A  // Only implemented sub-command 3 Read IF filter 
#define CAT_SET_TONE        0x1B  // Not implemented (VHF/UHF)
#define CAT_PTT             0x1C

/*
   CAT Sub COmmands
*/
// Mode Subcommand
#define CAT_MODE_LSB        0x00
#define CAT_MODE_USB        0x01
#define CAT_MODE_AM         0x02 // Not implemented
#define CAT_MODE_CW         0x03 // Not implemented
#define CAT_MODE_RTTY       0x04 // Not implemented
#define CAT_MODE_FM         0x05 // Not implemented
#define CAT_MODE_CW_R       0x06 // Not implemented
#define CAT_MODE_RTTY_R     0x07 // Not implemented
#define CAT_MODE_FILTER1    0x01 // Required for "read mode"

// VFO Subcommand
#define CAT_VFO_A           0x00
#define CAT_VFO_B           0x01
#define CAT_VFO_A_TO_B      0xA0
#define CAT_VFO_SWAP        0xB0

// Split Subcommand
#define CAT_SPLIT_OFF       0x00
#define CAT_SPLIT_ON        0x01
#define CAT_SIMPLE_DUP      0x10 // Not implemented
#define CAT_MINUS_DUP       0x11 // Not implemented
#define CAT_PLUS_DUP        0x12 // Not implemented

// S-Meter / Squelch Subcommand
#define CAT_READ_SUB_SQL    0x01 // Not implemented (squelch)
#define CAT_READ_SUB_SMETER 0x02

// PTT Subcommand
#define CAT_PTT_RX          0x00
#define CAT_PTT_TX          0x01

// 1A - MISC Subcommands
#define CAT_SET_MEM_CHAN    0x00  // Not implemented
#define CAT_SET_BANDSTACK   0x01  // Not implemented
#define CAT_SET_MEM_KEYER   0x02  // Not implemented
#define CAT_READ_IF_FILTER  0x03  // Hard coded response to keep WSJTX and other CAT controllers happy

// Command Receive States
#define CAT_RCV_WAITING     0  // waiting for 1st preamble byte
#define CAT_RCV_INIT        1  // waiting for 2nd preamble byte
#define CAT_RCV_RECEIVING   2  // waiting for command bytes

// Command buffer (without preamble and EOM)
// |FE|FE|56|E0|cmd|sub-cmd|data|FD|  // Preamble (FE) and EOM (FD) are discarded leaving
// 2 addr bytes , 1 command, 1 sub-command, up to 12 data, (longest is unimplemented edge frequency)
#define CAT_CMD_BUF_LENGTH  16

#define CAT_IX_TO_ADDR     0
#define CAT_IX_FROM_ADDR   1
#define CAT_IX_CMD         2
#define CAT_IX_SUB_CMD     3
#define CAT_IX_FREQ        3   // Set Freq has no sub-command
#define CAT_IX_MODE        3   // Get mode has no sub-command
#define CAT_IX_TUNE_STEP   3   // Get step has no sub-command
#define CAT_IX_ANT_SEL     3   // Get amt has no sub-command
#define CAT_IX_PTT         4   // PTT RX/TX indicator
#define CAT_IX_IF_FILTER   4   // IF Filter value
#define CAT_IX_SMETER      4   // S Meter 0-255
#define CAT_IX_SQUELCH     4   // Squelch 0=close, 1= open
#define CAT_IX_ID          5
#define CAT_IX_DATA        4   // Data following sub-comand

// Lentgth of commands that request data 
#define CAT_RD_LEN_NOSUB   3   //  3 bytes - 56 E0 cc
#define CAT_RD_LEN_SUB     4   //  4 bytes - 56 E0 cc ss  (cmd, sub command)

// Length of data responses
#define CAT_SZ_SMETER      6   //  6 bytes - E0 56 15 02 nn nn 
#define CAT_SZ_SQUELCH     5   //  5 bytes - E0 56 15 01 nn
#define CAT_SZ_PTT         5   //  5 bytes - E0 56 1C 00 nn
#define CAT_SZ_FREQ        8   //  8 bytes - E0 56 03 ff ff ff ff ff  (frequency in little endian BCD)
#define CAT_SZ_MODE        5   //  5 bytes - E0 56 04 mm ff  (mode, then filter)
#define CAT_SZ_IF_FILTER   5   //  5 bytes - E0 56 1A 03 nn
#define CAT_SZ_TUNE_STEP   4   //  4 bytes - E0 56 10 nn
#define CAT_SZ_ANT_SEL     4   //  4 bytes - E0 56 12 nn
#define CAT_SZ_ID          5   //  5 bytes - E0 56 19 00 56    (returns RIG ID)
#define CAT_SZ_UNIMP_1B    5   //  5 bytes - E0 56 NN SS 00    (unimplemented commands that require 1 data byte
#define CAT_SZ_UNIMP_2B    6   //  6 bytes - EO 56 NN SS 00 00 (unimplemented commandds that required 2 data bytes

typedef struct Rig746 {
    uint8_t * const cmdbuffer;
    uint8_t fsm_state;
    uint8_t cmdlen;
    const uint8_t maxlen;
} Rig746;


uint8_t ic746_get_command(void);
void ic746_parse_command(void);
void ic746_clean_command_buffer(void);
void ic746_send_echo(void);
void ic746_send_response(uint8_t cmdlen);
void ic746_send_nack(void);
void ic746_send_ack(void);

void ic746_add_handler_set_ptt(void (*userFunc)(uint8_t));
void ic746_add_handler_get_ptt(uint8_t (*userFunc)(void));
void ic746_add_handler_get_freq(long (*userFunc)(void));
void ic746_add_handler_set_freq(void (*userFunc)(long));
void ic746_add_handler_get_mode(uint8_t (*userFunc)(void));
void ic746_add_handler_set_mode(void (*userFunc)(uint8_t));
void ic746_add_handler_split(void (*userFunc)(uint8_t));
void ic746_add_handler_AtoB(void (*userFunc)(void));
void ic746_add_handler_swap_vfo(void (*userFunc)(void));
void ic746_add_handler_get_smeter(uint8_t(*userFunc)(void));
void ic746_add_handler_set_vfo(void (*userFunc)(uint8_t));

void doPtt(void);
void doSmeter(void);
void doSplit(void);
void doSetVfo(void);
void doSetFreq(void);
void doReadFreq(void);
void doSetMode(void); 
void doReadMode(void);
void doMisc(void);
void doReadId(void);
void doUnimplemented_1b(void);
void doUnimplemented_2b(void); 
void doTuneStep(void);
void doAntSel(void);

long BCDtoFreq(void);
void FreqtoBCD(long freq);
void SmetertoBCD(uint8_t s);

#endif
