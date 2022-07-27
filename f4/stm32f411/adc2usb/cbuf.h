#include <stdint.h>
typedef struct {
    int16_t * const buffer;
    int head;
    int tail;
    int length;
    const int maxlen;
} cbuf;


int buf_push(cbuf *c, int16_t data);
int buf_pop(cbuf *c, int16_t *data);
extern int16_t ringdata[256];
extern cbuf ring_buffer;
