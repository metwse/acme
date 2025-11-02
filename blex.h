/**
 * @file blex.h
 * @brief Lexer
 *
 * Boolean script lexical analyzer.
 */

#ifndef BLEX_H
#define BLEX_H

#include "bdef.h"


/* DO NOT CHANGE ORDER */
static const char *const b_tokens[] = {
	/* terminal symbols denoted by reserved words or symbols */
	"0", "1", "+", "*",
	"bool", "vec",
	"(", ")", "<", ">", "{", "}",
	";", "=",
	/* non-terminal tokens */
	"<positive-integer>",
};

enum b_token_type {
	TK_FALSE, TK_TRUE, TK_OR, TK_AND,
	TK_TY_BOOL, TY_TK_VEC,
	TK_LPAREN, TK_RPAREN, TK_LBRACKET, TK_RBRACKET, TK_LCURLY, TK_RCURLY,
	TK_DELIM, TK_ASGN,

	TK_POSITIVE_INT
};

/**
 * @brief Associated data with token.
 *
 * Extra information for a token.
 */
union b_seminfo {
	b_umem positive_integer;
};

struct b_token {
	union b_seminfo info;
	enum b_token_type token;
};


#endif
