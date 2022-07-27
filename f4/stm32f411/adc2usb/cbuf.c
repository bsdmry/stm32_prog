#include "cbuf.h"

int16_t ringdata[256] = {0};
cbuf ring_buffer = {
	.buffer = ringdata,
	.head = 0,
	.tail = 0,
	.length = 0,
	.maxlen = 256
};

int buf_push(cbuf *c, int16_t data)
{
    int next;

    next = c->head + 1;  // next is where head will point to after this write.
    if (next >= c->maxlen)
        next = 0;

    if (next == c->tail)  // if the head + 1 == tail, circular buffer is full
        return -1;

    c->buffer[c->head] = data;  // Load data and then move
    c->head = next;             // head to next data offset.
    if (c->head > c->tail){
	c->length = c->head - c->tail -1;
    } else {
	c->length = c->maxlen - c->tail + c-> head;
    }
    return 0;  // return success to indicate successful push.
}

int buf_pop(cbuf *c, int16_t *data)
{
    int next;

    if (c->head == c->tail)  // if the head == tail, we don't have any data
        return -1;

    next = c->tail + 1;  // next is where tail will point to after this read.
    if(next >= c->maxlen)
        next = 0;

    *data = c->buffer[c->tail];  // Read data and then move
    c->tail = next;              // tail to next offset.
    if (c->head > c->tail){
	c->length = c->head - c->tail -1;
    } else {
	c->length = c->maxlen - c->tail + c-> head;
    }
    return 0;  // return success to indicate successful push.
}

