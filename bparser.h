/**
 * @file bparser.h
 * @brief Parser
 *
 * Parse tree.
 */

#ifndef BPARSER_H
#define BPARSER_H

#include "bdef.h"


/**
 * SYMBOLS
 * ======================================================================
 */

/* -- general -- */

enum bbit {
	BBIT_TRUE,
	BBIT_FALSE,
};

struct bident {
	const char *ident;
	b_umem index; /* index for vectors */
};

struct bty {
	enum {
		BTY_BOOL,
		BTY_VEC,
	} type;
	b_umem size; /* for vectors */
};

/* -- expressions -- */

struct batom {
	enum {
		BATOM_EXPR,
		BATOM_INITLIST,
		BATOM_ASGNS,
		BATOM_CALL,
		BATOM_IDENT,
		BATOM_BIT,
	} kind;
	union {
		struct bexpr *expr;
		struct binitlist *initlist;
		struct basgns *asgns;
		struct bcall *call;
		struct bident *ident;
		enum bbit bit;
	} atom;
};

struct bfactor {
	enum {
		BFACTOR,
		BFACTOR_INVERSE,
	} kind;
	struct batom atom;
};

struct bterm {
	enum {
		BTERM_FACTOR,
		BTERM_AND,
	} kind;
	struct bterm *term; /* available in AND form */
	struct bfactor factor;
};

struct bexpr {
	enum {
		BEXPR_TERM,
		BEXPR_OR,
	} kind;
	struct bexpr *expr; /* available in OR form */
	struct bterm term;
};

struct binitlist {
	struct expr *exprs;
	b_umem size;
};

struct basgns {
	struct bident *asgns;
	b_umem size;
};

struct bcall {
	const char *ident;
	struct expr *params;
	b_umem size;
};

/* -- statements -- */

struct bdecl {
	struct bty type;
	const char *ident;
	struct basgns asgns;
};

struct bstmt {
	enum {
		BSTMT_DECL,
		BSTMT_ASGNS,
		BSTMT_EXPR,
		BSTMT_BLOCK,
	} kind;
	union {
		struct bdecl decl;
		struct basgns asgns;
		struct bexpr bexpr;
		struct {
			struct bstmt *stmts;
			b_umem size;
		} block_stmts;
	};
};

/**
 * ======================================================================
 * END SYMBOLS
 */

/* a Boolean script program */
struct b_prog {
	struct bstmt *stmts;
	b_umem stmt_count;
};


#endif
