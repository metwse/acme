/**
 * @file blex.h
 * @brief Lexer
 *
 * Boolean script lexical analyzer.
 */

#ifndef BLEX_H
#define BLEX_H

#include "bdef.h"
#include "bio.h"

#include <stdbool.h>


/* DO NOT CHANGE ORDER */
static const char *const b_tokens[] = {
	"<>" /* no more token */,

	/* terminal symbols denoted by reserved words or symbols */
	"0", "1",

	/* 1-character tokens */
	"+", "*", "'", ",", "=", ".",
	"(", ")", "<", ">", "[", "]", "{", "}",

	";" /* <no-escape-enter is also a delim> */,
	"bool", "vec",


	/* non-terminal tokens */
	"<positive-integer>", "<ident>",
};

enum b_token_type {
	NOTOKEN = 0,

	TK_FALSE, TK_TRUE,

	TK_OR, TK_AND, TK_INVOLUTION, TK_DELIM, TK_ASGN, TK_SUBSCRIPT,
	TK_L_PAREN, TK_R_PAREN,
	TK_L_ANGLE_BRACKET, TK_R_ANGLE_BRACKET,
	TK_L_BRACKET, TK_R_BRACKET,
	TK_L_CURLY, TK_R_CURLY,

	TK_STMT_DELIM,

	TK_TY_BOOL, TK_TY_VEC,

	TK_POSITIVE_INT, TK_IDENT
};

#define b_isterminal(token) ((token) < TK_POSITIVE_INT)

#define b_notok ((struct b_token) { .token = NOTOKEN })


/**
 * @brief Associated data with token.
 *
 * Extra information for a token.
 */
union b_seminfo {
	b_umem positive_integer;
	char *identifier;
};

struct b_token {
	union b_seminfo info;
	enum b_token_type token;
};

/**
 * @brief Lexer state
 *
 * State of the lexeme scanner.
 */
struct b_lex {
	// input stream
	struct bio *bio;
	char hold_char;
};


void b_lex_init(struct b_lex *);

/* returns the state of input stream  */
void *b_lex_clearinput(struct b_lex *);

/* returns the state of previous input */
void *b_lex_setinput(struct b_lex *, struct bio *);

/* first matched token */
struct b_token b_lex_next(struct b_lex *);


#endif
