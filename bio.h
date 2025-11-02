/**
 * @file bio.h
 * @brief Buffered streams
 *
 * Buffered I/O wrappers
 */

#ifndef BIO_H
#define BIO_H

#include "bdef.h"
#include "bmem.h"


#define B_EOF (-1)

typedef b_umem (*bio_reader)(struct b_buffer *buf, void *additional_data);

struct bio {
	// position in the `buf`
	b_umem p;

	// If null-termination of a slice replaces char in the buffer, the char
	// will be stored in this field to read next chunk.
	char hold_char;

	b_byte *prev_buf;
	struct b_buffer buf;

	// reader function
	bio_reader reader;
	// Additional state for use in reader
	void *reader_state;
};


void bio_init(struct bio *, bio_reader reader_func, void *aux_data);

/**
 * @returns the auxulary data of reader function.
 */
void *bio_destroy(struct bio *);

/**
 * Warning: Previuos reads may be invalidated after calling `bio_read` again.
 *
 * @returns a null-terminated char * up to n bytes.
 */
const char *bio_read(struct bio *, b_umem n);


#endif
