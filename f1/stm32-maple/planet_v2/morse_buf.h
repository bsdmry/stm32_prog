#include <inttypes.h>
#define BUF_SIZE 30
typedef struct {
  uint8_t *data;
  uint32_t size;
  uint32_t head;
  uint32_t tail;
  uint32_t length;
} mbuf;

uint8_t push(mbuf *ring, uint8_t data);
uint8_t pop(mbuf *ring, uint8_t *data);

extern mbuf morse_buf;
