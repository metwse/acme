/**
 * @file bmem.h
 * @brief Memory management interface
 *
 * Safe memory allocation wrappers.
 */

#ifndef BMEM_H
#define BMEM_H

#include "bdef.h"


struct b_buffer {
	b_ubyte *b;
	b_umem cap;
};


/* initializes a empty buffer */
void b_buffer_init(struct b_buffer *);

/* initializes a buffer with given cap */
void b_buffer_with_cap(struct b_buffer *, b_umem cap);

/* sets buffer capacity to 0 */
void b_buffer_reset(struct b_buffer *);

void b_buffer_resize(struct b_buffer *, b_umem cap);

static inline b_ubyte b_buffer_at(struct b_buffer buf, b_umem i)
{
	b_assert_expr("buffer out of boundary", i < buf.cap);

	return buf.b[i];
}

static inline char b_buffer_char_at(struct b_buffer buf, b_umem i)
{
	return (char) b_buffer_at(buf, i);
}

static inline void b_buffer_set(struct b_buffer buf, b_umem i, b_ubyte b)
{
	b_assert_expr("buffer out of boundary", i < buf.cap);

	buf.b[i] = b;
}

static inline void b_buffer_setall(struct b_buffer buf, b_ubyte b)
{
	for (b_umem i = 0; i < buf.cap; i++)
		buf.b[i] = b;
}


#endif
