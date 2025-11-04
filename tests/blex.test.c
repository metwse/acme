#include "../bdef.h"
#include "../bio.h"
#include "../blex.h"

#include <stdlib.h>
#include <string.h>

#include "mock_input_stream.h"


const struct b_token tokens[] = {
	{ .token = TK_IDENT, .info.ident = "long_identifier_aaaaaaaaaaaaaaaa", },
	{ .token = TK_ASGN, },
	{ .token = TK_TRUE, },
	{ .token = TK_STMT_DELIM, },

	{ .token = TK_NEWLINE, },


	{ .token = TK_TY_BOOL, },
	{ .token = TK_IDENT, .info.ident = "b", },
	{ .token = TK_ASGN, },
	{ .token = TK_IDENT, .info.ident = "a", },
	{ .token = TK_INVOLUTION, },
	{ .token = TK_STMT_DELIM, },

	{ .token = TK_IDENT, .info.ident = "c", },
	{ .token = TK_ASGN, },
	{ .token = TK_IDENT, .info.ident = "a", },
	{ .token = TK_AND, },
	{ .token = TK_IDENT, .info.ident = "b", },
	{ .token = TK_STMT_DELIM, },

	{ .token = TK_NEWLINE, },


	{ .token = TK_TY_VEC, },
	{ .token = TK_L_ANGLE_BRACKET, },
		{ .token = TK_POSITIVE_INT, .info.positive_int = 3, },
	{ .token = TK_R_ANGLE_BRACKET, },
	{ .token = TK_IDENT, .info.ident = "d", },
	{ .token = TK_ASGN, },
	{ .token = TK_L_BRACKET },
		{ .token = TK_L_PAREN },
			{ .token = TK_IDENT, .info.ident = "b", },
			{ .token = TK_AND, },
			{ .token = TK_FALSE, },
		{ .token = TK_R_PAREN },
		{ .token = TK_INVOLUTION },
	{ .token = TK_DELIM },
		{ .token = TK_TRUE },
		{ .token = TK_AND },
		{ .token = TK_L_PAREN },
			{ .token = TK_FALSE },
			{ .token = TK_OR },
			{ .token = TK_TRUE },
		{ .token = TK_R_PAREN },
	{ .token = TK_DELIM },
		{ .token = TK_L_PAREN },
			{ .token = TK_TRUE },
			{ .token = TK_OR },
			{ .token = TK_FALSE },
		{ .token = TK_R_PAREN },
		{ .token = TK_AND },
		{ .token = TK_TRUE },
	{ .token = TK_R_BRACKET },
	{ .token = TK_STMT_DELIM },

	{ .token = TK_NEWLINE, },


	{ .token = TK_TY_VEC, },
	{ .token = TK_L_ANGLE_BRACKET, },
		{ .token = TK_POSITIVE_INT, .info.positive_int = 1234567890, },
	{ .token = TK_R_ANGLE_BRACKET, },
	{ .token = TK_IDENT, .info.ident = "e", },
	{ .token = TK_STMT_DELIM },

	{ .token = TK_TY_VEC, },
	{ .token = TK_L_ANGLE_BRACKET, },
		{ .token = TK_POSITIVE_INT, .info.positive_int = 1, },
	{ .token = TK_R_ANGLE_BRACKET, },
	{ .token = TK_IDENT, .info.ident = "f", },
	{ .token = TK_STMT_DELIM },

	{ .token = NOTOKEN },
};

void consume_tokenstream(struct b_lex *lex, const char *tokenstream)
{
	struct bio bio;

	bio_init(&bio, mock_input_stream,
		 new_mock_input_stream_state(tokenstream,
					     strlen(tokenstream)));

	b_lex_setinput(lex, &bio);

	for (b_umem i = 0; i < sizeof(tokens) / sizeof(struct b_token); i++) {
		struct b_token lexeme;
		b_lex_next(lex, &lexeme);

		b_assert_expr(lexeme.token == tokens[i].token, "lexeme error");

		if (lexeme.token == TK_POSITIVE_INT) {
			b_assert_expr(
				lexeme.info.positive_int == tokens[i].info.positive_int,
				"int error"
			);
		} else if (lexeme.token == TK_IDENT) {
			b_assert_expr(
				strcmp(lexeme.info.ident, tokens[i].info.ident) == 0,
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

	struct b_token lexeme;
	enum b_lex_result res = b_lex_next(lex, &lexeme);

	b_assert_expr(expected_fail == res,
		      "expected notoken due to invalid token")

	free(b_lex_clearinput(lex));
}

int main()
{
	struct b_lex lex;

	b_lex_init(&lex);

	const char *tokenstream =
		"long_identifier_aaaaaaaaaaaaaaaa = 1;\n"
		"bool b = a'; c = a * b;\n"
		"vec<3> d = [(b * 0)', 1 * (0 + 1), (1 + 0) * 1];\n"
		"vec<1234567890> e; vec<1> f;";

	const char *invalid_tokenstreams[] = {
		"long_identifier_aaaaaaaaaaaaaaaaa also this tests memory leak in terminated streams",
		"12345678901",
		"ınvâlıd"
	};


	for (int _fuzz = 0; _fuzz < 4; _fuzz++)
		consume_tokenstream(&lex, tokenstream);


	test_invalid(&lex, invalid_tokenstreams[0], BLEXE_IDENT_TOO_LONG);
	test_invalid(&lex, invalid_tokenstreams[1], BLEXE_INTEGER_TOO_LARGE);
	test_invalid(&lex, invalid_tokenstreams[2], BLEXE_NO_MATCH);
}
