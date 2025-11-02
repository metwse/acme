#include "bdef.h"
#include "bio.h"
#include "bmem.h"

#include <stdlib.h>
#include <string.h>


void bio_init(struct bio *bio, bio_reader reader, void *state)
{
	bio->p = 0;
	b_buffer_init(&bio->buf);

	bio->prev_buf = NULL;
	bio->hold_char = 0;
	bio->reader = reader;
	bio->reader_state = state;
}

void *bio_destroy(struct bio *bio)
{
	b_buffer_reset(&bio->buf);

	return bio->reader_state;
}

const char *bio_read(struct bio *bio, b_umem n)
{
	if (bio->prev_buf) {
		free(bio->prev_buf);
		bio->prev_buf = NULL;
	}

	// restore the character that was in its place before null-terminator
	if (bio->p < bio->buf.cap && bio->hold_char) {
		b_buffer_set(bio->buf, bio->p, bio->hold_char);
		bio->hold_char = 0;
	}

	// if read buffer already enough for next n bytes, just return a slice
	// of it.
	if (bio->p + n < bio->buf.cap) {
		char *res = (char *) &bio->buf.b[bio->p];

		bio->p += n;

		bio->hold_char = b_buffer_at(bio->buf, bio->p);
		b_buffer_set(bio->buf, bio->p, '\0');

		return res;
	}

	char *res = malloc(n + 1);
	b_assert_expr("nomem", res);

	res[n] = '\0';

	// special condition that the buffer has read enough bytes but it do
	// not have enough memory store extra null-termiator
	// TODO: optimize using already allocated memory, i.e. shifting one
	// byte leftwards
	if (bio->p + n == bio->buf.cap) {
		memcpy(res, bio->buf.b + bio->p, n);

		bio->p = 0;
		b_buffer_reset(&bio->buf);

		bio->prev_buf = res;
		return res;
	}

	b_umem total_bytes_read = 0;
	while (total_bytes_read != n) {
		if (bio->p == bio->buf.cap) {
			bio->p = 0;
			b_buffer_reset(&bio->buf);

			if (bio->reader(&bio->buf, bio->reader_state) == 0) {
				// no more data
				// shrink result buffer to read size

				res = realloc(res, total_bytes_read + 1);
				b_assert_expr("unexpected NULL", res);

				res[total_bytes_read] = '\0';

				b_buffer_reset(&bio->buf);

				break;
			}
		}

		res[total_bytes_read] = b_buffer_at(bio->buf, bio->p);
		total_bytes_read++;
		bio->p++;
	}

	if (total_bytes_read == 0) {
		bio->prev_buf = NULL;
		free(res);
		return NULL;
	} else {
		bio->prev_buf = res;
		return res;
	}
}
