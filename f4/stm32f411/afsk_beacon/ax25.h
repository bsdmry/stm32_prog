#ifndef H_AX25
#define H_AX25
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

typedef struct {
        uint8_t rcvrCallsign[6];
        uint8_t rcvrSSID;
        uint8_t sndrCallsign[6];
        uint8_t sndrSSID;
        uint8_t repeater1Callsign[6];
        uint8_t repeater1SSID;
        uint8_t repeater2Callsign[6];
        uint8_t repeater2SSID;
        uint8_t ctrlField;
        uint8_t pidField;
        char* payload;
        uint16_t payloadSize;
} aprs_msg;

uint16_t ax25CalcFrameSize(aprs_msg* aprsMsg);
void ax25structToArray(aprs_msg* aprsMsg, uint8_t* array);

void setAddress(uint8_t* ptrCallsingField, uint8_t* ptrSSIDField, char* callsign, uint8_t ssid, uint8_t isLastAddress);
void setReciever(aprs_msg* msg, char* callsign, uint8_t ssid);
void setSender(aprs_msg* msg, char* callsign, uint8_t ssid, uint8_t isLastAddress);
void setRepeater1(aprs_msg* msg, char* callsign, uint8_t ssid, uint8_t isLastAddress);
void setRepeater2(aprs_msg* msg, char* callsign, uint8_t ssid, uint8_t isLastAddress);
void setCtrlField(aprs_msg* msg);
void setPidField(aprs_msg* msg);
void setPayload(aprs_msg* msg, char* payload);


#endif


