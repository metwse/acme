#include "bdef.h"
#include "bgrammar.h"

#include <stdbool.h>


#define TK(tk) { .ty = BSYMBOL_TOKEN, .tk_ty = BTK_ ## tk }
#define NT(nt) { .ty = BSYMBOL_NONTERMINAL, .nt_ty = BNT_ ## nt }

#define IEOB -1 /* integer representing EOB */
#define IEOC -2 /* integer representing EOC */

#define EOB (const struct production) { .end = IEOB } /* end of body */
#define EOC { { .end = IEOC } } /* end of construct */

// maximum number of variants constructing the same nonterminal
#define MAX_VARIANT_COUNT 7
// maximum body length of a rule
#define MAX_BODY_LENGTH 4

/** nonterminal rules */
struct production {
	enum bsymbol_type ty /** whether the symbol is a terminal or not */;
	union {
		enum btk_type tk_ty;
		enum bnt_type nt_ty;
		int end;
	};
};

/** right-recursive list rules */
#define RR(head, listelem, delim) \
	{ { NT(listelem), NT(head ## _REST), EOB }, EOC }, \
	{ { TK(delim), NT(head), NT(head ## _REST), EOB }, { EOB }, EOC }



static const struct production
productions[BNONTERMINAL_COUNT][MAX_VARIANT_COUNT][MAX_BODY_LENGTH + 1 /* +1 for EOB */] = {
	/* BIT */ { { TK(TRUE), EOB }, { TK(FALSE), EOB }, EOC },
	/* POSITIVE_INT */ { { TK(POSITIVE_INT), EOB }, EOC },

	/* IDENT */ {
		{ TK(IDENT), EOB },
		EOC
	},
	/* IDENT_OR_MEMBER */ {
		{ NT(IDENT), EOB },
		{ NT(IDENT), TK(SUBSCRIPT), NT(POSITIVE_INT), EOB },
		EOC
	},

	/* TY */ { { NT(TY_BOOL), EOB }, { NT(TY_VEC), EOB }, EOC },
	/* TY_BOOL */ {
		{ TK(TY_BOOL), EOB },
		{ EOB },
		EOC
	},
	/* TY_VEC */ {
		{ TK(TY_VEC), TK(L_ANGLE_BRACKET), NT(POSITIVE_INT), TK(R_ANGLE_BRACKET), EOB },
		EOC
	},

	/* EXPR_OR_INITLIST */ {
		{ NT(INITLIST), EOB },
		{ NT(EXPR), EOB },
		EOC
	},
	/* EXPR */ RR(EXPR, TERM, OR),
	/* INITLIST */ {
		{ TK(L_BRACKET), NT(EXPR_LS), TK(R_BRACKET), EOB },
		EOC
	},

	/* TERM */ RR(TERM, FACTOR, AND),
	/* FACTOR */ {
		{ NT(ATOM), NT(OPTINVOLUTION), EOB },
		EOC
	},
	/* ATOM */ {
		{ TK(L_PAREN), NT(EXPR), TK(R_PAREN), EOB },
		{ TK(L_PAREN), NT(ASGN_BOOL), TK(R_PAREN), EOB },
		{ NT(CALL), EOB },
		{ NT(IDENT_OR_MEMBER), EOB },
		{ NT(BIT), EOB },
		EOC
	},
	/* OPTINVOLUTION */ {
		{ TK(INVOLUTION), EOB },
		{ EOB },
		EOC
	},
	/* CALL */ {
		{ NT(IDENT), TK(L_PAREN), NT(OPTPARAMS), TK(R_PAREN), EOB },
		EOC
	},
	/* PARAMS */ RR(PARAMS, EXPR_OR_INITLIST, DELIM),
	/* OPTPARAMS */ {
		{ NT(PARAMS), EOB },
		{ EOB },
		EOC
	},

	/* STMT */ {
		{ NT(DECL_BOOL), TK(STMT_DELIM), EOB },
		{ NT(DECL_VEC), TK(STMT_DELIM), EOB },
		{ NT(ASGN_BOOL), TK(STMT_DELIM), EOB },
		{ NT(ASGN_VEC), TK(STMT_DELIM), EOB },
		{ NT(CALL), TK(STMT_DELIM), EOB },
		{ TK(L_CURLY), NT(STMTS), TK(R_CURLY), EOB },
		EOC
	},
	/* STMTS */ {
		{ NT(STMT), NT(STMTS), EOB },
		{ EOB },
		EOC
	},

	/* DECL_BOOL */ {
		{ NT(TY_BOOL), NT(IDENT_LS), NT(DECL_BOOL_OPTASGN), EOB },
		EOC
	},
	/* DECL_BOOL_OPTASGN */ {
		{ TK(ASGN), NT(ASGN_BOOL), EOB },
		{ TK(ASGN), NT(EXPR), EOB },
		{ EOB },
		EOC
	},
	/* DECL_VEC */ {
		{ NT(TY_VEC), NT(IDENT_LS), NT(DECL_VEC_OPTASGN), EOB },
		EOC
	},
	/* DECL_VEC_OPTASGN */ {
		{ TK(ASGN), NT(ASGN_VEC), EOB },
		{ TK(ASGN), NT(INITLIST), EOB },
		{ EOB },
		EOC
	},

	// TODO: deduplicate asgn rule definitions
	/* ASGN */ {
		{ NT(IDENT_OR_MEMBER_LS), NT(ASGN_BOOL_REST), TK(ASGN), NT(EXPR_OR_INITLIST), EOB },
		EOC
	},
	/* ASGN_REST */ {
		{ TK(ASGN), NT(IDENT_OR_MEMBER_LS), NT(ASGN_REST), EOB },
		{ EOB },
		EOC
	},
	/* ASGN_BOOL */ {
		{ NT(IDENT_OR_MEMBER_LS), NT(ASGN_BOOL_REST), TK(ASGN), NT(EXPR), EOB },
		EOC
	},
	/* ASGN_BOOL_REST */ {
		{ TK(ASGN), NT(IDENT_OR_MEMBER_LS), NT(ASGN_BOOL_REST), EOB },
		{ EOB },
		EOC
	},
	/* ASGN_VEC */ {
		{ NT(IDENT_OR_MEMBER_LS), NT(ASGN_VEC_REST), TK(ASGN), NT(INITLIST_LS), EOB },
		EOC
	},
	/* ASGN_VEC_REST */ {
		{ TK(ASGN), NT(IDENT_OR_MEMBER_LS), NT(ASGN_VEC_REST), EOB },
		{ EOB },
		EOC
	},

	/* IDENT_LS */ RR(IDENT_LS, IDENT, DELIM),
	/* IDENT_OR_MEMBER_LS_LS */ RR(IDENT_OR_MEMBER_LS, IDENT, DELIM),

	/* EXPR_LS */ RR(EXPR_LS, EXPR, DELIM),
	/* INITLIST_LS */ RR(INITLIST_LS, INITLIST, DELIM),
};

/* no lock/once used lazy initialization for widest compability */
__attribute__((unused)) static inline b_umem child_cap_of(enum bnt_type nt)
{
	static b_umem child_caps[BNONTERMINAL_COUNT];
	bool initialized = false;

	if (!initialized) {
		for (b_umem i = 0; i < BNONTERMINAL_COUNT; i++) {
			b_umem len;
			child_caps[i] = 0;

			for (b_umem j = 0; j < MAX_VARIANT_COUNT; j++) {
				if (productions[i][j][0].end == IEOC)
					break;

				for (len = 0; len < MAX_BODY_LENGTH; len++)
					if (productions[i][j][len].end == IEOB)
						break;

				if (len > child_caps[i])
					child_caps[i] = len;
			}
		}

		initialized = true;
	}

	return child_caps[nt];
}
