/**
 * @file bparser.h
 * @brief Boolean script parser
 *
 * Parsed node definitions and syntax-directed translators
 */

#ifndef BPARSER_H
#define BPARSER_H


#include "bdef.h"
#include "blex.h"


/** @brief parse tree node */
struct b_pt_node {
	enum {
		BP_BIT,
		BP_POSITIVE_INT,

		BP_IDENT, BP_IDENT_OR_MEMBER,

		BP_TY,


		BP_EXPR, BP_EXPR_REST,
		BP_TERM, BP_FACTOR_REST,
		BP_ATOM,

		BP_CALL,
		BP_OPTPARAMS,


		BP_STMT,
		BP_STMTS,

		BP_DECL,
		BP_DECL_OPTASGNS,


		BP_ASGNS, BP_ASGNS_REST,

		BP_IDENT_LS, BP_IDENT_LS_REST,
		BP_IDENT_OR_MEMBER_LS, BP_IDENT_OR_MEMBER_LS_REST,

		BP_EXPR_LS, BP_EXPR_LS_REST,
	} kind /** node kind */;
	union b_seminfo seminfo /** additional semantic info */;
	struct b_pt_node *nodes /** child nodes */;
	b_umem node_count /** child node count */;
};


#endif
