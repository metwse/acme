#include "bdef.h"
#include "blex.h"
#include "bio.h"

#include <stdlib.h>
#include <string.h>


const b_mem MAX_TOKEN_LEN = b_max(BLEX_MAX_IDENT_LEN,
				  BLEX_MAX_POSITIVE_INT_LEN);

void b_lex_init(struct b_lex *lex)
{
	lex->bio = NULL;
	lex->peek = 0;
	lex->lookahead = NOTOKEN;
}

void *b_lex_clearinput(struct b_lex *lex)
{
	void *old_state = NULL;

	if (lex->bio)
		old_state = bio_destroy(lex->bio);

	lex->peek = 0;
	lex->bio = NULL;
	lex->lookahead = NOTOKEN;

	return old_state;
}

void *b_lex_setinput(struct b_lex *lex, struct bio *bio)
{
	void *old_state = b_lex_clearinput(lex);

	lex->bio = bio;

	return old_state;
}

/* match next lexeme without lookahead filtering */
enum b_lex_result simple_next(struct b_lex *lex, struct b_token *out)
{
	out->token = NOTOKEN;

	char token_string[MAX_TOKEN_LEN + 1];

	// SIGNED, due to token_len--
	b_mem token_len;

	bool valid_ident = true, valid_positive_int = true;

	// extra length of 1 is for detecting invalid size violating tokens
	/* token_len: length of saved chars for token */
	// token_len + 1 is used for sake of clarity
	for (token_len = 0; token_len <= MAX_TOKEN_LEN + 1; token_len++) {
		char input;

		if (lex->peek) {
			input = lex->peek;
			lex->peek = 0;
		} else {
			const char *input_ = bio_read(lex->bio, 1);

			if (!input_)
				break;

			input = input_[0];
		}

		if (input == ' ' || input == '\t') {
			if (token_len + 1 == 1) {
				token_len--;
				continue;
			} else {
				break;
			}
		}

		for (int i = TK_OR; i <= TK_STMT_DELIM; i++)
			if (input == b_tokens[i][0]) {
				if (token_len == 0) {
					out->token = i;
					return BLEXOK;
				} else {
					// One-character "punctuation" can
					// break an identifier or integer read.
					// Hold the character for the next
					// lexeme.
					lex->peek = input;
					break;
				}
			}

		// transfer break from inner loop TK_OR -> TK_STMT_DELIM to
		// outer for loop.
		if (lex->peek)
			break;

		token_string[token_len] = input;
		token_string[token_len + 1] = '\0';

		for (int i = TK_TY_BOOL; i <= TK_TY_VEC; i++) {
			if (strcmp(b_tokens[i], token_string) == 0) {
				out->token = i;
				return BLEXOK;
			}
		}

		if (token_len == 0 && input == '0') {
			valid_ident = valid_positive_int = false;
		} else {
			if (!('0' <= input && input <= '9'))
				valid_positive_int = false;

			if (!(
				('0' <= input && input <= '9') ||
				('a' <= input && input <= 'z') ||
				('A' <= input && input <= 'Z') ||
				input == '_'
			))
				valid_ident = false;

			if (!valid_ident && !valid_positive_int) {
				lex->peek = input;
				break;
			}
		}
	}

	if (token_len == 1 && token_string[0] == b_tokens[TK_FALSE][0]) {
		out->token = TK_FALSE;
		return BLEXOK;
	}

	// DO NOT REORDER valid_positive_int -> valid_ident check order
	//
	// valid_ident variable is true for positive integers
	if (token_len && valid_positive_int) {
		// Bit "1" in subset of positive_integer tokens.
		if (token_len == 1 && token_string[0] == b_tokens[TK_TRUE][0]) {
			out->token = TK_TRUE;
			return BLEXOK;
		}

		if (token_len <= BLEX_MAX_POSITIVE_INT_LEN) {
			out->token = TK_POSITIVE_INT;
			out->info.positive_int = strtoull(token_string,
							  NULL, 10);
			return BLEXOK;
		} else {
			return BLEXE_INTEGER_TOO_LARGE;
		}
	} else if (token_len && valid_ident) {
		if (token_len <= BLEX_MAX_IDENT_LEN) {
			char *ident = malloc(sizeof(char) * (token_len + 1));
			memcpy(ident, token_string, token_len + 1 /* +1 for \0 */);

			out->token = TK_IDENT;
			out->info.ident = ident;
			return BLEXOK;
		} else {
			return BLEXE_IDENT_TOO_LONG;
		}
	}

	return BLEXE_NO_MATCH;
}

enum b_lex_result b_lex_next(struct b_lex *lex, struct b_token *out) {
	enum b_lex_result res = simple_next(lex, out);

	if (res == BLEXOK) {
		if (lex->lookahead == TK_L_ANGLE_BRACKET) {
			if (out->token == TK_TRUE) {
				out->token = TK_POSITIVE_INT;
				out->info.positive_int = 1;
			}
		}
		lex->lookahead = out->token;
	} else {
		lex->lookahead = NOTOKEN;
	}

	return res;
}
