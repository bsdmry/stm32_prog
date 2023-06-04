#ifndef H_HDLC
#define H_HDLC
#include "ax25.h"

typedef struct {
        uint16_t frameSize;
        uint8_t* frameData;
} hdlc_frame;

uint16_t calcBitstuffedArrayLength(uint16_t orig_size);
void appendBitstuffedByte(uint8_t* BitsuffArray, uint8_t* newByte, uint16_t* bitStuffArrayIndex, uint8_t* newByteBitIndex);
uint16_t makeBitstuffArray(uint8_t* msg, uint16_t msg_len, uint8_t* bsmsg);

void hdlcCalcCRC(uint8_t* data, uint16_t data_len);
hdlc_frame* hdlcMakeFrame(aprs_msg* aprsMsg);

#endif
