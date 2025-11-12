#include "bdef.h"
#include "bgrammar.h"

#include <stdbool.h>


#define EOB -1 /* integer representing EOB */
#define EOC -2 /* integer representing EOC */

// maximum number of variants constructing the same nonterminal
#define MAX_VARIANT_COUNT 7
// maximum body length of a rule, +1 for EOB
#define MAX_BODY_LENGTH 11


/** nonterminal rules */
struct production {
	enum bsymbol_type ty /** whether the symbol is a terminal or not */;
	union {
		enum btk_type tk_ty;
		enum bnt_type nt_ty;
		int end;
	};
};


#define SEOB (const struct production) { .end = EOB } /* end of body */
#define SEOC { { .end = EOC } } /* end of construct */

#define TK(tk) { .ty = BSYMBOL_TOKEN, .tk_ty = BTK_ ## tk }
#define NT(nt) { .ty = BSYMBOL_NONTERMINAL, .nt_ty = BNT_ ## nt }


/** rule */
#define r(...) { { __VA_ARGS__ SEOB }, SEOC }

/** right-recursive list rules */
#define rrr(head, listelem, delim) \
	{ { NT(listelem), NT(head ## _REST), SEOB }, SEOC }, \
	{ { TK(delim), NT(head), NT(head ## _REST), SEOB }, { SEOB }, SEOC }

/** optional rules */
#define ropt(...) { { __VA_ARGS__, SEOB }, { SEOB }, SEOC }

/** array delimiter for the sake of clarity */
#define or SEOB, }, {


static const struct production
productions[BNONTERMINAL_COUNT][MAX_VARIANT_COUNT][MAX_BODY_LENGTH] = {
// ===== primitive types ======================================================
	/* <bit> ::= */ r(
		TK(TRUE),
	or	TK(FALSE),
	),
	/* <positive_int> ::= */ r(
		TK(POSITIVE_INT),
	),

// ===== identifiers ==========================================================
	/* <ident> ::= */ r(
		TK(IDENT),
	),
	/* <ident_or_member> ::= */ r(
		NT(IDENT),
	or	NT(IDENT), TK(DOT), NT(POSITIVE_INT),
	),

// ===== types ================================================================
	/* <ty> ::= */ r(
		NT(TY_BOOL),
	or	NT(TY_VEC),
	),
	/* <ty_bool> ::= */ r(
		TK(TY_BOOL),
	or	/* empty */
	),
	/* <ty_vec> ::= */ r(
		TK(TY_VEC), TK(LANGLE_BRACKET), NT(POSITIVE_INT), TK(RANGLE_BRACKET),
	),

	/* <expr_or_initlist> ::= */ r(
		NT(INITLIST),
	or	NT(EXPR),
	),
	/* <initlist> ::= */ r(
		TK(LBRACKET), NT(EXPR_LS), TK(RBRACKET),
	),

// ===== expressions ==========================================================
	/* <expr> ::= */
		rrr(EXPR, TERM, OR),
	/* <term> ::= */
		rrr(TERM, FACTOR, AND),
	/* <factor> ::= */ r(
		NT(ATOM), NT(OPTINVOLUTION),
	),
	/* <atom> ::= */ r(
		TK(LPAREN), NT(EXPR), TK(RPAREN),
	or	TK(LPAREN), NT(ASGN), TK(RPAREN),
	or	NT(CALL),
	or	NT(IDENT_OR_MEMBER),
	or	NT(BIT),
	),
	/* <optinvolution> ::= */ ropt(TK(INVOLUTION)),
	/* <call> ::= */ r(
		NT(IDENT), TK(LPAREN), NT(CALL_OPTPARAMS), TK(RPAREN),
	),
	/* <call_params> ::= */
		rrr(CALL_PARAMS, EXPR_OR_INITLIST, COMMA),
	/* <call_optparams> ::= */
		ropt(NT(CALL_PARAMS)),

// ===== statements ===========================================================
	/* <stmt> ::= */ r(
		NT(VAR_DECL), TK(SEMI),
	or	NT(EXPR_DECL), TK(SEMI),
	or	NT(ASGN), TK(SEMI),
	or	NT(CALL), TK(SEMI),
	or	TK(LCURLY), NT(STMTS), TK(RCURLY),
	or	TK(SEMI),
	),
	/* <stmts> ::= */ ropt(NT(STMT), NT(STMTS)),

// ===== variable or expression declarations ==================================
	/* <var_decl> ::= */ r(
		NT(TY), NT(IDENT_LS), NT(VAR_DECL_OPTASGN),
	),
	/* <var_decl_optasgn> ::= */ r(
		TK(EQ), NT(ASGN),
	or	TK(EQ), NT(EXPR_OR_INITLIST_LS),
	or	/* empty */
	),
	/* <expr_decl> ::= */ r(
		// "expr" <ident> "(" <expr_decl_optparams> ")"
		TK(EXPR), NT(IDENT), TK(LPAREN), NT(EXPR_DECL_OPTPARAMS), TK(RPAREN),
			// "->" "(" <expr_decl_optparams> ")"
			TK(RARROW), TK(LPAREN), NT(EXPR_DECL_OPTPARAMS), TK(RPAREN),
			NT(STMTS),
	),
	/* <expr_decl_param> ::= */ r(
		NT(TY), NT(IDENT),
	),
	/* <expr_decl_params> ::= */
		rrr(EXPR_DECL_PARAMS, EXPR_DECL_PARAM, COMMA),
	/* <expr_decl_optparams> ::= */
		ropt(NT(EXPR_DECL_PARAMS)),

// ===== right-recursive list types ===========================================
	/* <asgn> ::= */ r(
		NT(IDENT_OR_MEMBER_LS), NT(ASGN_REST), TK(EQ), NT(EXPR_OR_INITLIST_LS),
	),
	/* <asgn_rest> ::= */
		ropt(TK(EQ), NT(IDENT_OR_MEMBER_LS), NT(ASGN_REST)),
	/* <ident_ls> ::= */
		rrr(IDENT_LS, IDENT, COMMA),
	/* <ident_or_member_ls> ::= */
		rrr(IDENT_OR_MEMBER_LS, IDENT_OR_MEMBER, COMMA),
	/* <expr_ls> ::= */
		rrr(EXPR_LS, EXPR, COMMA),
	/* <expr_or_initlist_ls> ::= */
		rrr(EXPR_OR_INITLIST_LS, EXPR_OR_INITLIST, COMMA),
};


/* no lock/once used lazy initialization for widest compability */
static inline b_umem b_unused_fn(child_cap_of)(enum bnt_type nt)
{
	static b_umem child_caps[BNONTERMINAL_COUNT];
	bool initialized = false;

	if (!initialized) {
		for (b_umem i = 0; i < BNONTERMINAL_COUNT; i++) {
			b_umem len;
			child_caps[i] = 0;

			for (b_umem j = 0; j < MAX_VARIANT_COUNT; j++) {
				if (productions[i][j][0].end == EOC)
					break;

				for (len = 0; len < MAX_BODY_LENGTH; len++)
					if (productions[i][j][len].end == EOB)
						break;

				if (len > child_caps[i])
					child_caps[i] = len;
			}
		}

		initialized = true;
	}

	return child_caps[nt];
}

/* no lock/once used lazy initialization for widest compability */
static inline b_umem b_unused_fn(variant_count_of)(enum bnt_type nt)
{
	static b_umem variant_counts[BNONTERMINAL_COUNT];
	bool initialized = false;

	if (!initialized) {
		for (b_umem i = 0; i < BNONTERMINAL_COUNT; i++) {
			b_umem count = 0;

			for (b_umem j = 0;
			     productions[i][j][0].end != EOC && j < MAX_VARIANT_COUNT;
			     j++)
				count++;

			variant_counts[i] = count;
		}

		initialized = true;
	}

	return variant_counts[nt];
}

#undef SEOB
#undef SEOC

#undef TK
#undef NT

#undef r
#undef rrr
#undef ropt

#undef or
