#include "../bdef.h"
#include "../bgrammar.h"
#include "../bio.h"
#include "../blex.h"

#include <stdlib.h>
#include <string.h>

#include "mock_input_stream.h"

#include "./examples/simple_tokenized.c"


void test_valid(struct b_lex *lex, struct test_case tc)
{
	struct bio bio;

	bio_init(&bio, mock_input_stream,
		 new_mock_input_stream_state(tc.str, strlen(tc.str)));

	b_lex_setinput(lex, &bio);

	for (b_umem i = 0; i < sizeof(tc.tk) / sizeof(struct btoken); i++) {
		struct btoken lexeme;
		b_lex_next(lex, &lexeme);

		if (lexeme.ty == BTK_NOTOKEN)
			break;

		b_assert_expr(lexeme.ty == tc.tk[i].ty, "lexeme error");

		if (lexeme.ty == BTK_POSITIVE_INT) {
			b_assert_expr(
				lexeme.info.positive_int == tc.tk[i].info.positive_int,
				"int error"
			);
		} else if (lexeme.ty == BTK_IDENT) {
			b_assert_expr(
				strcmp(lexeme.info.ident, tc.tk[i].info.ident) == 0,
				"ident error"
			);

			free(lexeme.info.ident);
		}
	}

	free(b_lex_clearinput(lex));
}

void test_invalid(struct b_lex *lex, const char *invalid_tokenstream,
		  enum b_lex_result expected_fail)
{
	struct bio bio;

	bio_init(&bio, mock_input_stream,
		 new_mock_input_stream_state(invalid_tokenstream,
					     strlen(invalid_tokenstream)));

	free(b_lex_setinput(lex, &bio));

	struct btoken lexeme;
	enum b_lex_result res = b_lex_next(lex, &lexeme);

	b_assert_expr(expected_fail == res,
		      "expected notoken due to invalid token");

	free(b_lex_clearinput(lex));
}

int main()
{
	struct b_lex lex;

	b_lex_init(&lex);

	for (b_umem i = 0; i < sizeof(test_cases) / sizeof(struct test_case); i++)
		test_valid(&lex, test_cases[i]);

	const char *invalid_tokenstreams[] = {
		"long_identifier_aaaaaaaaaaaaaaaaa also this tests memory leak in terminated streams",
		"12345678901",
		"ınvâlıd",
		"a-a",
		"a-",
		"\\not newline",
	};

	test_invalid(&lex, invalid_tokenstreams[0], BLEXE_IDENT_TOO_LONG);
	test_invalid(&lex, invalid_tokenstreams[1], BLEXE_INTEGER_TOO_LARGE);
	test_invalid(&lex, invalid_tokenstreams[2], BLEXE_NO_MATCH);
	test_invalid(&lex, invalid_tokenstreams[3], BLEXE_NO_MATCH);
	test_invalid(&lex, invalid_tokenstreams[4], BLEXE_NO_MATCH);
	test_invalid(&lex, invalid_tokenstreams[5], BLEXE_INVALID_ESCAPE_CHAR);
}
