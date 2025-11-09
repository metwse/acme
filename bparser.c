#include "bdef.h"
#include "bgrammar.h"
#include "bio.h"
#include "blex.h"
#include "bparser.h"

#include <stdlib.h>

#include "detail/bparser.h"
#include "grammar.c"


void b_parser_init(struct b_parser *p)
{
	p->token_count = 0;
	p->tokens = NULL;
	p->cur = p->root = NULL;
	b_lex_init(&p->lex);
}

void *b_parser_clearinput(struct b_parser *p)
{
	if (p->token_count)
		free(p->tokens);
	p->token_count = 0;
	p->tokens = NULL;

	return b_lex_clearinput(&p->lex);
}

void *b_parser_setinput(struct b_parser *p, struct bio *bio)
{
	if (p->token_count)
		free(p->tokens);
	p->token_count = 0;
	p->tokens = NULL;

	return b_lex_setinput(&p->lex, bio);
}

struct bsymbol *new_nt_node(struct bsymbol *parent,
			    enum bnt_type ty)
{
	struct bsymbol *n = malloc(sizeof(struct bsymbol));
	b_assert_expr(n, "nomem");

	n->parent = parent;
	if (parent)
		push_child(parent, n);

	n->ty = BSYMBOL_NONTERMINAL;

	b_umem child_cap = child_cap_of(ty);
	n->nt.ty = ty;
	n->nt.child_count = 0;
	n->nt.variant = 0;
	b_assert_expr(child_cap, "a nonterminal with no children is not meaningful");
	b_assert_expr((
		n->nt.children =
			malloc(sizeof(struct bsymbol *) * child_cap)
	), "nomem");

	return n;
}

struct bsymbol *new_tk_node(struct bsymbol *parent,
			    enum btk_type ty)
{
	struct bsymbol *n = malloc(sizeof(struct bsymbol));
	b_assert_expr(n, "nomem");

	n->parent = parent;
	if (parent)
		push_child(parent, n);

	n->ty = BSYMBOL_TOKEN;

	n->tk.ty = ty;

	return n;
}

void push_child(struct bsymbol *parent, struct bsymbol *child)
{
	b_assert_expr(parent->ty == BSYMBOL_NONTERMINAL,
		      "nonterminal parent expected");

	b_assert_expr(parent->nt.child_count < child_cap_of(parent->nt.ty),
		      "child capacity of parent is exceeded");

	parent->nt.children[parent->nt.child_count++] = child;
}

/* teardown the tree into tokens. returns constructed the tree. */
void teardown_tree(struct bsymbol *sym, struct btoken **out, b_umem *out_len)
{
	if (sym->ty == BSYMBOL_NONTERMINAL) {
		for (b_umem i = 0; i < sym->nt.child_count; i++)
			teardown_tree(sym->nt.children[i], out, out_len);

		if (sym->nt.children)
			free(sym->nt.children);
	} else {
		if (*out == NULL) {
			*out = malloc(sizeof(struct btoken));
			*out_len = 0;
		} else {
			*out = realloc(*out,
				       sizeof(struct btoken) * (*out_len + 1));
		}
		b_assert_expr(*out, "nomem");

		(*out)[*out_len] = sym->tk;
		(*out_len)++;
	}
	free(sym);
}

void teardown_children(struct bsymbol *sym, struct btoken **out, b_umem *out_len)
{
	for (b_umem i = 0; i < sym->nt.child_count; i++)
		teardown_tree(sym->nt.children[i], out, out_len);

	sym->nt.child_count = 0;
}

struct btoken next_token(struct b_parser *p)
{
	if (p->token_count == 0) {
		struct btoken out;

		b_lex_next(&p->lex, &out);

		return out;
	} else {
		return p->tokens[--p->token_count];
	}
}

enum b_parser_result b_parser_try_next(struct b_parser *p, struct bsymbol *out)
{
	if (p->root == NULL) {
		p->cur = p->root = new_nt_node(NULL, BNT_STMT);
	}

	struct bsymbol *sym = p->cur;

	struct btoken tk;

	while ((tk = next_token(p)).ty != BTK_NOTOKEN) {
		while (1) {
			struct production rule =
				productions[sym->nt.ty][sym->nt.variant][sym->nt.child_count];

			if (rule.end == EOC) {
				if (sym->parent != NULL) {
					sym = sym->parent;

					teardown_children(sym, &p->tokens, &p->token_count);
					sym->nt.variant++;

					continue;
				} else {
					break;
				}
			}

			if (rule.end == EOB) {
				sym = sym->parent;
				if (sym == NULL)
					break;

				continue;
			}

			if (rule.ty == BSYMBOL_TOKEN && tk.ty == rule.tk_ty) {
				new_tk_node(sym, tk.ty)->tk.info = tk.info;
			} else if (rule.ty == BSYMBOL_NONTERMINAL) {
				sym = new_nt_node(sym, rule.nt_ty);
			} else {
				teardown_children(sym, &p->tokens, &p->token_count);
				sym->nt.variant++;
			}
		}
	}

	*out = *p->root;

	return BPARSER_READY;
}
