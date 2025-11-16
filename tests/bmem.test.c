#include "../bmem.h"

#include <stdlib.h>
#include <time.h>


int main()
{
	srand(time(NULL));

	struct b_buffer buf;

	for (int i = 0; i < 256; i++) {
		b_buffer_with_cap(&buf, i);

		for (int j = 0; j < i; j++)
			*(b_buffer_at(buf, j)) = i;

		for (int j = 0; j < i; j++) {
			b_assert_expr(*b_buffer_at(buf, j) == i,
				      "faulty element_at");
			*(b_buffer_at(buf, j)) = 255 - j;
		}

		for (int j = 0; j < i; j++)
			b_assert_expr(*b_buffer_at(buf, j) == 255 - j,
				      "faulty element_at");

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

	struct b_stack s;

	b_stack_init(&s);

	for (int i = 0; i < 32; i++)
		b_stack_push(&s, i);

	for (int i = 0; i < 32; i++) {
		b_assert_expr(b_stack_peek(&s) == 31 - i, "faulty peek");
		b_assert_expr(b_stack_pop(&s) == 31 - i, "faulty pop");
	}

	b_stack_reset(&s);

	b_stack_with_cap(&s, 1);

	for (int i = 0; i < 32; i++)
		b_stack_resize(&s, i);

	b_stack_reset(&s);

	for (b_mem i = -128; i < 1024; i++) {
		b_stack_push_elem(&s, &i, sizeof(i));
	}

	for (b_mem i = 1023; i >= -128; i--) {
		b_mem peek = *cast(int *, b_stack_peek_elem(&s, sizeof(i)));
		b_mem out = *cast(int *, b_stack_pop_elem(&s, sizeof(i)));

		b_assert_expr(peek == i, "faulty peek_elem");
		b_assert_expr(out == i, "faulty pop_elem");
	}

	b_stack_reset(&s);
}
