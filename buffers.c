
// io/data ring buffer functions & defs

// generic I/O buffer control structure
struct ringbuf {
	uint8_t size;	// must always be an even 2^N number.
	volatile uint8_t r_ptr;		// read pointer
	volatile uint8_t w_ptr;		// write pointer
};

void buffer_init(struct ringbuf *buf, uint8_t size) {
	buf->size = size-1 ;
	buf->r_ptr = 0;
	buf->w_ptr = 0;
}

// The following functions take these arguments:
//	1) pointer to buffer control struct

inline uint8_t is_writeable(struct ringbuf *buf) {
	// SREG &= 0b01111111;
	// see if buffer is writeable (ie r_ptr != w_ptr)
	return ( (buf->r_ptr-buf->w_ptr-1 ) & buf->size );
	// SREG |= 0b10000000;
}

inline uint8_t is_readable(struct ringbuf *buf) {
	// SREG &= 0b01111111;
	// buffer readable? 
	return ( (buf->w_ptr-buf->r_ptr) & buf->size );
	// SREG |= 0b10000000;
}

// The following functions take these arguments:
//	1) pointer to buffer control struct
//	2) pointer to actual buffer element 0
//	3) byte or word to write

inline void write_byte(struct ringbuf *buf, uint8_t *array, uint8_t byte) {
	// buf->buffer[buf->w_ptr] = byte;
	array[buf->w_ptr] = byte; 
	buf->w_ptr = (buf->w_ptr + 1) & buf->size;
}

inline void write_word(struct ringbuf *buf, uint16_t *array, uint16_t word) {
	// buf->buffer[buf->w_ptr] = byte;
	array[buf->w_ptr] = word;
	buf->w_ptr = (buf->w_ptr + 1) & buf->size;
}

// The following functions take these arguments:
//	1) pointer to buffer control struct
//	2) pointer to actual buffer element 0

inline uint8_t read_byte(struct ringbuf *buf, uint8_t *array) {
	// uint8_t value = buf->buffer[buf->r_ptr];
	volatile uint8_t value = array[buf->r_ptr];
	buf->r_ptr = (buf->r_ptr + 1) & buf->size;
	return(value);
}

inline uint16_t read_word(struct ringbuf *buf, uint16_t *array) {
	// uint8_t value = buf->buffer[buf->r_ptr];
	volatile uint16_t value = array[buf->r_ptr];
	buf->r_ptr = (buf->r_ptr + 1) & buf->size;
	return(value);
}


