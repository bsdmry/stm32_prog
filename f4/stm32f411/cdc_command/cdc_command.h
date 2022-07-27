#include "cbuf.h"
#include "usb_cdc.h"

#define CAT_CMD_BUF_LENGTH  16

#define CAT_RCV_WAITING     0  // waiting for 1st preamble byte
#define CAT_RCV_INIT        1  // waiting for 2nd preamble byte
#define CAT_RCV_RECEIVING   2  // waiting for command bytes

typedef struct Rig746 {
    uint8_t * const cmdbuffer;
    uint8_t fsm_state;
    uint8_t cmdlen;
    const uint8_t maxlen;
} Rig746;
