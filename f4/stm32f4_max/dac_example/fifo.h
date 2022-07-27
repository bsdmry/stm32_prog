typedef enum {
	FIFO_OK,
	FIFO_OVERRUN,
	FIFO_UNDERRUN
} FifoActionStatus;

typedef struct {
        uint16_t size;
        uint16_t head;
        uint16_t count;
        uint32_t* buff;

} FIFO_buffer;

FIFO_buffer* fifo_init(uint16_t fifo_size);
FifoActionStatus fifo_put(FIFO_buffer *fifo, uint32_t *data);
FifoActionStatus fifo_get(FIFO_buffer *fifo, uint32_t *data);
