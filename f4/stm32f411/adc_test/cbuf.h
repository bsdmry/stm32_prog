#include <stdint.h>
typedef struct {
    int16_t * const buffer;
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint8_t length;
    const uint8_t maxlen;
} cbuf;


int buf_push(cbuf *c, int16_t data);
int buf_pop(cbuf *c, int16_t *data);
extern int16_t ringdata[64];
extern cbuf ring_buffer;

