#include "cbuf.h"

int8_t buf_push_u8(cbuf_u8 *c, uint8_t data){
    uint16_t next;
    next = c->head + 1;
    if (next > c->maxindex) { next = 0; }
    if (next == c->tail) { return -1;}
    c->buffer[c->head] = data;
    c->head = next;
    if (c->head >= c->tail) { c->length = c->head - c->tail; } else { c->length = c->maxindex - c->tail + c-> head + 1; }
    return 0;
}

int8_t buf_pop_u8(cbuf_u8 *c, uint8_t *data){
    uint16_t next;
    if (c->head == c->tail) { return -1; }
    next = c->tail + 1;
    if(next > c->maxindex) { next = 0; }
    *data = c->buffer[c->tail];
    c->tail = next;
    if (c ->head >= c->tail) { c->length = (c->head - c->tail); } else { c->length = c->maxindex - c->tail + c->head + 1;}
    return 0;
}

int8_t buf_push_u16(cbuf_u16 *c, uint16_t data){
    uint16_t next;
    next = c->head + 1;
    if (next > c->maxindex) { next = 0; }
    if (next == c->tail) { return -1;}
    c->buffer[c->head] = data;
    c->head = next;
    if (c->head >= c->tail) { c->length = c->head - c->tail; } else { c->length = c->maxindex - c->tail + c-> head + 1; }
    return 0;
}

int8_t buf_pop_u16(cbuf_u16 *c, uint16_t *data){
    uint16_t next;
    if (c->head == c->tail) { return -1; }
    next = c->tail + 1;
    if(next > c->maxindex) { next = 0; }
    *data = c->buffer[c->tail];
    c->tail = next;
    if (c ->head >= c->tail) { c->length = (c->head - c->tail); } else { c->length = c->maxindex - c->tail + c->head + 1;}
    return 0;
}

int8_t buf_push_s16(cbuf_s16 *c, int16_t data){
    uint16_t next;
    next = c->head + 1;
    if (next > c->maxindex) { next = 0; }
    if (next == c->tail) { return -1;}
    c->buffer[c->head] = data;
    c->head = next;
    if (c->head >= c->tail) { c->length = c->head - c->tail; } else { c->length = c->maxindex - c->tail + c-> head + 1; }
    return 0;
}

int8_t buf_pop_s16(cbuf_s16 *c, int16_t *data){
    uint16_t next;
    if (c->head == c->tail) { return -1; }
    next = c->tail + 1;
    if(next > c->maxindex) { next = 0; }
    *data = c->buffer[c->tail];
    c->tail = next;
    if (c ->head >= c->tail) { c->length = (c->head - c->tail); } else { c->length = c->maxindex - c->tail + c->head + 1;}
    return 0;
}
