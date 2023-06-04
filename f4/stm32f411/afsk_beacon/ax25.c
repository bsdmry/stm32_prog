#include "ax25.h"

void setAddress(uint8_t* ptrCallsingField, uint8_t* ptrSSIDField, char* callsign, uint8_t ssid, uint8_t isLastAddress)
{
        uint8_t szCallSign = strlen(callsign);
        for (uint8_t i = 0; i < 6; i++){
                if (i < szCallSign){
                        ptrCallsingField[i] = (((uint8_t)callsign[i]) << 1);
                } else {
                        ptrCallsingField[i] = 0x40;
                }
                //print_byte_hex(ptrCallsingField[i]);
        }
        ptrSSIDField[0] = ((0x70 ^ (ssid & 0x0F)) << 1);
        if (isLastAddress){
                ptrSSIDField[0] |= (1<<0);
        }

}

void setReciever(aprs_msg* msg, char* callsign, uint8_t ssid){
        setAddress(msg->rcvrCallsign, &msg->rcvrSSID, callsign, ssid, 0);
}
void setSender(aprs_msg* msg, char* callsign, uint8_t ssid, uint8_t isLastAddress){
        setAddress(msg->sndrCallsign, &msg->sndrSSID, callsign, ssid, isLastAddress);
}

void setRepeater1(aprs_msg* msg, char* callsign, uint8_t ssid, uint8_t isLastAddress){
        setAddress(msg->repeater1Callsign, &msg->repeater1SSID, callsign, ssid, isLastAddress);
}

void setRepeater2(aprs_msg* msg, char* callsign, uint8_t ssid, uint8_t isLastAddress){
        setAddress(msg->repeater2Callsign, &msg->repeater2SSID, callsign, ssid, isLastAddress);
}


void setCtrlField(aprs_msg* msg)
{
        msg->ctrlField = 0x03;
}
void setPidField(aprs_msg* msg)
{
        msg->pidField = 0xF0;
}
void setPayload(aprs_msg* msg, char* payload)
{
        uint8_t szPayload = strlen(payload);
        msg->payloadSize = szPayload;
        msg->payload = malloc(sizeof(unsigned char)*szPayload);
        for (uint8_t i = 0; i < szPayload; i++){
                msg->payload[i] = payload[i];
        }
}

uint16_t ax25CalcFrameSize(aprs_msg* aprsMsg)
{
        uint16_t size = 16; //Sender + Reciver addresses + Control byte + PID byte
        if (aprsMsg->repeater1Callsign[0] != 0) { size += 7; }
        if (aprsMsg->repeater2Callsign[0] != 0) { size += 7; }
        size += aprsMsg->payloadSize;
        return size;
}

void ax25structToArray(aprs_msg* aprsMsg, uint8_t* array){
        uint16_t writeIndex = 0;
        memcpy(array, aprsMsg, 14);
        writeIndex = 14;
        if (aprsMsg->repeater1Callsign[0] != 0) {
                memcpy(array+writeIndex, aprsMsg->repeater1Callsign, 6);
                writeIndex += 6;
                memcpy(array+writeIndex, &aprsMsg->repeater1SSID, 1);
                writeIndex++;
        }
        if (aprsMsg->repeater2Callsign[0] != 0) {
                memcpy(array+writeIndex, aprsMsg->repeater2Callsign, 6);
                writeIndex += 6;
                memcpy(array+writeIndex, &aprsMsg->repeater2SSID, 1);
                writeIndex++;
        }
        memcpy(array+writeIndex, &aprsMsg->ctrlField, 1);
        writeIndex++;
        memcpy(array+writeIndex, &aprsMsg->pidField, 1);
        writeIndex++;
        memcpy(array+writeIndex, aprsMsg->payload, aprsMsg->payloadSize);
        //free(aprsMsg->payload);
        //aprsMsg->payloadSize = 0;
}


