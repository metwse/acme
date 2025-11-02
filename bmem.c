#include "bmem.h"
#include "bdef.h"

#include <stdlib.h>


void b_buffer_init(struct b_buffer *buf)
{
	buf->cap = 0;
	buf->b = NULL;
}

void b_buffer_with_cap(struct b_buffer *buf, b_umem cap)
{
	buf->cap = cap;

	if (cap) {
		buf->b = malloc(cap);

		b_assert_expr("malloc should not return NULL", buf->b);
	} else {
		buf->b = NULL;
	}
}

/* set buffer capacity to 0 */
void b_buffer_reset(struct b_buffer *buf)
{
	if (buf->cap) {
		free(buf->b);

		buf->b = NULL;
		buf->cap = 0;
	}
}

void b_buffer_resize(struct b_buffer *buf, b_umem cap)
{
	if (cap == 0)
		return b_buffer_reset(buf);

	buf->b = buf->cap == 0 ? malloc(cap) : realloc(buf->b, cap);
	buf->cap = cap;

	b_assert_expr("realloc should not return NULL", buf->b);
}
