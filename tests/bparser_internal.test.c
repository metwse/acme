// tests for bparser's internal functions

#include "../bdef.h"
#include "../bgrammar.h"

#include <stdlib.h>

#include "testutil.h"

#include "../detail/bparser.h"


int main()
{
	struct btoken tk[] = {
		TK(SEMI),
		TK(RBRACKET),
		TK(FALSE),
		TK(COMMA),
		TK(TRUE),
		TK(LBRACKET),
		TK(EQ),
		TK_DETAIL(IDENT, ident, "a"),
		TK(RANGLE_BRACKET),
		TK_DETAIL(POSITIVE_INT, positive_int, 2),
		TK(LANGLE_BRACKET),
		TK(TY_VEC),
	};

	struct bsymbol *stmts = new_nt_node(NULL, BNT_STMTS);
	struct bsymbol *stmt = new_nt_node(stmts, BNT_STMT);
	new_nt_node(stmts, BNT_STMTS);

	struct bsymbol *var_decl = new_nt_node(stmt, BNT_VAR_DECL);
	new_tk_node(stmt, BTK_SEMI);

	struct bsymbol *ty = new_nt_node(var_decl, BNT_TY);
	struct bsymbol *ident_ls = new_nt_node(var_decl, BNT_IDENT_LS);
	struct bsymbol *var_decl_optasgn = new_nt_node(var_decl, BNT_VAR_DECL_OPTASGN);

// ==== left side of the declaration ==========================================
	struct bsymbol *ty_vec = new_nt_node(ty, BNT_TY_VEC);
	new_tk_node(ty_vec, BTK_TY_VEC);
	new_tk_node(ty_vec, BTK_LANGLE_BRACKET);
	struct bsymbol *positive_int = new_nt_node(ty_vec, BNT_POSITIVE_INT);
	new_tk_node(positive_int, BTK_POSITIVE_INT)->tk.info.positive_int = 2;
	new_tk_node(ty_vec, BTK_RANGLE_BRACKET);

	struct bsymbol *ident = new_nt_node(ident_ls, BNT_IDENT_LS);
	new_tk_node(ident, BTK_IDENT)->tk.info.ident = "a";
	new_nt_node(ident_ls, BNT_IDENT_LS_REST);

// ==== right side ============================================================
	new_tk_node(var_decl_optasgn, BTK_EQ);
	struct bsymbol *expr_or_initlist_ls = new_nt_node(var_decl_optasgn, BNT_EXPR_OR_INITLIST_LS);

	struct bsymbol *expr_or_initlist = new_nt_node(expr_or_initlist_ls, BNT_EXPR_OR_INITLIST);
	new_nt_node(expr_or_initlist_ls, BNT_EXPR_OR_INITLIST_LS_REST);

	struct bsymbol *initlist = new_nt_node(expr_or_initlist, BNT_INITLIST);
	new_tk_node(initlist, BTK_LBRACKET);
	struct bsymbol *expr_ls = new_nt_node(initlist, BNT_EXPR_LS);
	new_tk_node(initlist, BTK_RBRACKET);

	// ==== 1st element in initlist
	struct bsymbol *expr1 = new_nt_node(expr_ls, BNT_EXPR);
	struct bsymbol *expr1_term = new_nt_node(expr1, BNT_TERM);
	new_nt_node(expr1, BNT_EXPR_REST);

	struct bsymbol *expr1_factor = new_nt_node(expr1_term, BNT_FACTOR);
	new_nt_node(expr1_term, BNT_TERM_REST);

	struct bsymbol *expr1_atom = new_nt_node(expr1_factor, BNT_ATOM);
	new_nt_node(expr1_factor, BNT_OPTINVOLUTION);

	struct bsymbol *expr1_bit = new_nt_node(expr1_atom, BNT_BIT);
	new_tk_node(expr1_bit, BTK_TRUE);

	// ==== 2nd element in initlist
	struct bsymbol *expr_ls_rest = new_nt_node(expr_ls, BNT_EXPR_LS_REST);
	new_tk_node(expr_ls_rest, BTK_COMMA);
	struct bsymbol *expr2 = new_nt_node(expr_ls_rest, BNT_EXPR);
	new_nt_node(expr_ls_rest, BNT_EXPR_LS_REST);

	struct bsymbol *expr2_term = new_nt_node(expr2, BNT_TERM);
	new_nt_node(expr2, BNT_EXPR_REST);

	struct bsymbol *expr2_factor = new_nt_node(expr2_term, BNT_FACTOR);
	new_nt_node(expr2_term, BNT_TERM_REST);

	struct bsymbol *expr2_atom = new_nt_node(expr2_factor, BNT_ATOM);
	new_nt_node(expr2_factor, BNT_OPTINVOLUTION);

	struct bsymbol *expr2_bit = new_nt_node(expr2_atom, BNT_BIT);
	new_tk_node(expr2_bit, BTK_FALSE);

	struct b_parser p;
	b_parser_init(&p);
	teardown_tree_postorder(&p, stmts);

	for (b_umem i = 0; i < sizeof(tk) / sizeof(struct btoken); i++) {
		b_assert_expr(tk[i].ty == p.tokens[i].ty, "tree teardown failed");
	}

	free(p.tokens);
}
