#ifndef STROP_H
#define STROP_H
#include <stdint.h>
uint32_t hexstr2val(char *str, uint8_t len);
void val2hexstr(uint32_t val, char* str, uint8_t len);
#endif
