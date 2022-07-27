#include "fifo.h"

FIFO_buffer* fifo_init(uint16_t fifo_size){
	FIFO_buffer* fifo = malloc(sizeof(FIFO_buffer));
	fifo->head = 0;
	fifo->count = 0;
	fifo->size = fifo_size;
	fifo->buff = malloc(sizeof(uint32_t) * fifo_size);
	return fifo;
}

FifoActionStatus fifo_put(FIFO_buffer *fifo, uint32_t *data){
	if (fifo->count == fifo->size){
		return FifoActionStatus.FIFO_OVERRUN;
	}
	uint16_t tail = fifo->head + fifo->count;
	if (tail >= fifo->size){
		tail -= fifo->size;
	}
	fifo->buff[tail] = data;
	fifo->count++;
	return FIFO_OK;	
}
FifoActionStatus fifo_get(FIFO_buffer *fifo, uint32_t *data){
	if (fifo-> count == 0){
		return FifoActionStatus.FIFO_UNDERRUN;
	}
	data = fifo->buff[fifo->head];
	fifo->head++;
	if (fifo->head >= fifo->size){
		fifo->head -= fifo->size;
	}
	fifo->count--;
	return FIFO_OK;
}
