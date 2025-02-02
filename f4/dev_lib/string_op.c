#include "string_op.h"

uint32_t hexstr2val(char *str, uint8_t len){
    uint32_t r = 0;
    for(uint8_t i = 0; i < len; i++){
        uint8_t c = (uint8_t)str[i];
        uint8_t val = 0;
        if (c >= 0x30 && c <= 0x39){ //0-0
            val = c - 0x30;
        } else if (c >= 0x41 && c <= 0x46){ //A-F
            val = c - 0x37;
        } else { //a-f
            val = c- 0x57;
        }
        r = r << 4;
        r = r | val;
    }
    return r;
}

void val2hexstr(uint32_t val, char* str, uint8_t len){
    for (uint8_t i = len; i != 0; i--){
        uint8_t d = (uint8_t)(val & 0x0F);
        val = val >> 4;
        char c = '0';
        if (d > 9){
            c = (char)(d + 0x37);
        } else {
            c = (char)(d + 0x30);
        }
        str[i-1] = c;
    }
}
