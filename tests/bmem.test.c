#include "../bmem.h"

#include <stdlib.h>
#include <time.h>


int main()
{
	srand(time(NULL));

	struct b_buffer buf;

	for (int i = 0; i < 256; i++) {
		b_buffer_with_cap(&buf, i);

		b_buffer_setall(buf, i);

		for (int j = 0; j < i; j++) {
			b_assert_expr("", b_buffer_at(buf, j) == i);
			b_buffer_set(buf, j, 255 - j);
		}

		for (int j = 0; j < i; j++)
			b_assert_expr("", b_buffer_at(buf, j) == 255 - j);

		b_buffer_reset(&buf);

		b_buffer_resize(&buf, 256 - i);
		b_buffer_reset(&buf);
	}

	for (int i = 0; i < 1024; i++) {
		int size = rand() % 128;
		int new_size = rand() % 128;

		b_buffer_with_cap(&buf, size);
		b_buffer_resize(&buf, new_size);

		b_buffer_reset(&buf);
	}
}
