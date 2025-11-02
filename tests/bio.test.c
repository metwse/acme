#include "../bdef.h"
#include "../bio.h"
#include "../bmem.h"

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>


struct streamreader_state {
	// position in the stream
	b_umem p;
	b_umem len;
	const char *stream;
};


b_umem streamreader(struct b_buffer *buf, void *state_) {
	struct streamreader_state *state = state_;

	b_umem chunk_size = rand() % 1024 + 1;

	if (state->p + chunk_size > state->len)
		chunk_size = state->len - state->p;

	if (chunk_size == 0)
		return 0;

	b_buffer_with_cap(buf, chunk_size);
	memcpy(buf->b, state->stream + state->p, chunk_size);

	state->p += chunk_size;

	return chunk_size;
}

struct streamreader_state *new_state(const char *stream, b_umem len) {
	struct streamreader_state *state = malloc(
		sizeof(struct streamreader_state)
	);

	state->p = 0;
	state->len = len;
	state->stream = stream;

	return state;
}

void test(b_umem stream_size, b_umem read_limit) {
	struct bio bio;

	// initial data
	char *stream = malloc(sizeof(char) * stream_size);

	for (b_umem i = 0; i < stream_size; i++)
		// fill random data
		stream[i] = rand() % 254 + 1;

	bio_init(&bio, streamreader, new_state(stream, stream_size));

	// null-terminated read output
	char *collect = malloc(sizeof(char) * (stream_size + 1));
	char *collect_cursor = collect;

	while (true) {
		// number of bytes trying to read
		b_umem wants_read = rand() % read_limit + 1;

		const char *data = bio_read(&bio, wants_read);

		if (!data)
			break;

		b_assert_expr("data is not null-terminated",
			      strlen(data) <= wants_read);

		strcpy(collect_cursor, data);
		collect_cursor += wants_read;
	}

	b_assert_expr("collected data differs from initial data",
		      memcmp(collect, stream, stream_size) == 0);

	free(collect);
	free(stream);

	free(bio_destroy(&bio));
}

int main()
{
	srand(time(NULL));

	for (int _fuzz = 0; _fuzz < 512; _fuzz++) {
		for (int i = 1; i < 8; i++) {
		b_umem stream_size = (rand() % 5 + 1) << i;
			for (int j = 1; j < 8; j++) {
				b_umem read_limit = (rand() % 5 + 1) << j;

				test(stream_size, read_limit);
			}
		}
	}
}
