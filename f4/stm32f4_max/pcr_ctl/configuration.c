#include "configuration.h"
int32_t frequency = MIN_FREQ;
int16_t volume = 0;
int16_t squelch = 0;


void set_frequency(int32_t f){
    frequency = f;
}
int32_t get_frequency(void){
    return frequency;
}
void inc_frequency(int32_t step){
    if ((frequency+step) <= MAX_FREQ){
        frequency = frequency + step;
    } else {
        frequency = MAX_FREQ;
    }
}
void dec_frequency(int32_t step){
    if ((frequency-step) >= MIN_FREQ){
        frequency = frequency - step;
    } else {
        frequency = MIN_FREQ;
    }
}

void set_volume(int16_t v){
    volume = v;
}
int16_t get_volume(void){
    return volume;
}
void inc_volume(int16_t step){
    if ((volume+step) <= 255){
        volume = volume + step;
    } else {
        volume = 255;
    }
}
void dec_volume(int16_t step){
    if ((volume-step) <= 0){
        volume = volume - step;
    } else {
        volume = 0;
    }
}

void set_squelch(int16_t s){
    squelch = s;
}
int16_t get_squelch(void){
    return squelch;
}
void inc_squelch(int16_t step){
    if ((squelch+step) <= 255){
        squelch = squelch + step;
    } else {
        squelch = 255;
    }
}
void dec_squelch(int16_t step){
    if ((squelch-step) <= 0){
        squelch = squelch - step;
    } else {
        squelch = 0;
    }
}
