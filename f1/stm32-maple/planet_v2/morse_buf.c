#include "morse_buf.h"

uint8_t buffer[BUF_SIZE] = {};
 
mbuf morse_buf = {
  .data = buffer,
  .size = sizeof(buffer),
  .head = 0,
  .tail = 0,
  .length = 0
};

uint8_t push(mbuf *ring, uint8_t data)
{
  if (((ring->tail + 1) % ring->size) != ring->head)
  {
    ring->data[ring->tail++] = data;
    ring->tail %= ring->size;
    ring->length++;
    return 1;
  }
  return 0;
}

uint8_t pop(mbuf *ring, uint8_t *data)
{
  if (ring->head != ring->tail)
  {
    *data = ring->data[ring->head++];
    ring->head %= ring->size;
    ring->length--;
    return 1;
  }
  return 0;
}
