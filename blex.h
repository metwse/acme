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


/* DO NOT CHANGE THE ORDER */
static const char *const b_tokens[] = {
	"<>" /* no more token */,

	/* terminal symbols denoted by reserved words or symbols */
	"0", "1",

	/* 1-character tokens */
	"+", "*", "'", ",", "=", ".",
	"(", ")", "<", ">", "[", "]", "{", "}",

	"\\", "\n", ";",

	/* keywords and reserved words */
	"bool", "vec",


	/* non-terminal tokens */
	"<positive-int>", "<ident>",
};

enum b_token_type {
	NOTOKEN = 0,

	TK_FALSE, TK_TRUE,

	TK_OR, TK_AND, TK_INVOLUTION, TK_DELIM, TK_ASGN, TK_SUBSCRIPT,
	TK_L_PAREN, TK_R_PAREN,
	TK_L_ANGLE_BRACKET, TK_R_ANGLE_BRACKET,
	TK_L_BRACKET, TK_R_BRACKET,
	TK_L_CURLY, TK_R_CURLY,

	TK_ESCAPE, TK_NEWLINE, TK_STMT_DELIM,

	TK_TY_BOOL, TK_TY_VEC,

	TK_POSITIVE_INT, TK_IDENT
};

#define b_isterminal(token) ((token) < TK_POSITIVE_INT)

#define b_notok ((struct b_token) { .token = NOTOKEN })

#define BLEX_MAX_IDENT_LEN 32

#define BLEX_MAX_POSITIVE_INT_LEN 10


/**
 * @brief Associated data with token.
 *
 * Extra information for a token.
 */
union b_seminfo {
	b_umem positive_int;
	char *ident;
};

struct b_token {
	union b_seminfo info;
	enum b_token_type token;
};

/* lexer result */
enum b_lex_result {
	BLEXOK = 0,
	BLEXE_IDENT_TOO_LONG, /* identifier longer than BLEX_MAX_IDENT_LEN */
	BLEXE_INTEGER_TOO_LARGE, /* integer string longer than BLEX_MAX_POSITIVE_INT_LEN */
	BLEXE_NO_MATCH, /* could not match token */
};

/**
 * @brief Lexer state
 *
 * State of the lexeme scanner.
 */
struct b_lex {
	struct bio *bio; /* input stream */
	char peek;
	enum b_token_type lookahead;
};


void b_lex_init(struct b_lex *);

/* returns the state of input stream  */
void *b_lex_clearinput(struct b_lex *);

/* returns the state of previous input */
void *b_lex_setinput(struct b_lex *, struct bio *);

/* first matched token */
enum b_lex_result b_lex_next(struct b_lex *, struct b_token *);


#endif
