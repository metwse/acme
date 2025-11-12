#include "bdef.h"
#include "bgrammar.h"
#include "bio.h"
#include "blex.h"
#include "bparser.h"

#include <stdbool.h>
#include <stdlib.h>

#include "detail/bparser.h"
#include "grammar.c"


#define get_body(sym) productions[sym->nt.ty][sym->nt.variant]
#define get_current_rule(sym) get_body(sym)[sym->nt.child_count]

#define is_grammar_complete(sym) get_current_rule(sym).end == EOB
#define is_construct_end(sym) get_body(sym)[0].end == EOC


void b_parser_init(struct b_parser *p)
{
	p->token_count = 0;
	p->tokens = NULL;
	p->cur = p->root = NULL;
	b_lex_init(&p->lex);
}

void *b_parser_clearinput(struct b_parser *p)
{
	if (p->tokens)
		free(p->tokens);

	p->token_count = 0;
	p->tokens = NULL;
	p->cur = p->root = NULL;

	return b_lex_clearinput(&p->lex);
}

void *b_parser_setinput(struct b_parser *p, struct bio *bio)
{
	return b_lex_setinput(&p->lex, bio);
}

void push_child(struct bsymbol *parent, struct bsymbol *child)
{
	b_assert_expr(parent->ty == BSYMBOL_NONTERMINAL,
		      "nonterminal parent expected");

	b_assert_expr(parent->nt.child_count < child_cap_of(parent->nt.ty),
		      "child capacity of parent is exceeded");

	parent->nt.children[parent->nt.child_count++] = child;
}

void restore_token(struct b_parser *p, struct btoken tk)
{
	if (p->tokens == NULL) {
		p->tokens = malloc(sizeof(struct btoken));
		p->token_count = 0;
	} else {
		p->tokens = realloc(p->tokens,
				    sizeof(struct btoken) * (p->token_count + 1));
	}
	b_assert_expr(p->tokens, "nomem");

	p->tokens[p->token_count] = tk;
	p->token_count++;
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

void next_variant(struct b_parser *p)
{
	while (p->cur->nt.child_count) {
		struct bsymbol *next_cur = NULL;

		for (int i = p->cur->nt.child_count; i > 0; i--) {
			struct bsymbol *child = p->cur->nt.children[i - 1];

			if (child->ty == BSYMBOL_NONTERMINAL) {
				next_cur = child;
				break;
			} else {
				p->cur->nt.child_count--;

				restore_token(p, child->tk);
			}
		}

		if (next_cur)
			p->cur = next_cur;
		else
			break;
	}

	for (b_umem i = p->cur->nt.child_count; i > 0; i--)
		teardown_tree_preorder(p, p->cur->nt.children[i - 1]);

	p->cur->nt.variant++;
	p->cur->nt.child_count = 0;
}

struct btoken next_token(struct b_parser *p)
{
	struct btoken out;

	if (p->token_count == 0) {
		b_lex_next(&p->lex, &out);
	} else {
		out = p->tokens[--p->token_count];

		p->tokens = realloc(p->tokens,
				    sizeof(struct btoken) * p->token_count);
	}
	return out;
}

void teardown_tree_postorder(struct b_parser *p, struct bsymbol *sym)
{
	if (sym->ty == BSYMBOL_NONTERMINAL) {
		for (b_umem i = sym->nt.child_count; i > 0; i--)
			teardown_tree_postorder(p, sym->nt.children[i - 1]);

		if (sym->nt.children)
			free(sym->nt.children);
	} else {
		restore_token(p, sym->tk);
	}
	free(sym);
}

void teardown_tree_preorder(struct b_parser *p, struct bsymbol *sym)
{
	if (sym->ty == BSYMBOL_NONTERMINAL) {
		for (b_umem i = 0; i < sym->nt.child_count; i++)
			teardown_tree_preorder(p, sym->nt.children[i]);

		if (sym->nt.children)
			free(sym->nt.children);
	} else {
		restore_token(p, sym->tk);
	}
	free(sym);
}

void backtrace(struct b_parser *p)
{
	struct bsymbol *parent = p->cur->parent;

	// index of cur in parent
	b_umem this_i;

	// begin bottom-up backtracing
	for (this_i = 0; this_i < parent->nt.child_count; this_i++) {
		if (parent->nt.children[this_i] == p->cur)
			break;
	}

	for (b_umem i = parent->nt.child_count; i > this_i; i--)
		teardown_tree_postorder(p, parent->nt.children[i - 1]);
	parent->nt.child_count = this_i;

	p->cur = parent;
	// end

	next_variant(p);

	if (is_construct_end(p->cur))
		backtrace(p);
}

enum feed_result {
	READY,
	CONTINUE,
	RESTORE,
	NOMATCH,
} feed_parser(struct b_parser *p, struct btoken tk)
{
	if (is_grammar_complete(p->cur)) {
		p->cur = p->cur->parent;

		if (p->cur == NULL)
			return READY;
		else
			return RESTORE;
	}

	if (is_construct_end(p->cur)) {
		restore_token(p, tk);

		backtrace(p);

		return CONTINUE;
	}

	struct production rule = get_current_rule(p->cur);

	switch (rule.ty) {
	case BSYMBOL_TOKEN:
		if (rule.tk_ty == tk.ty) {
			new_tk_node(p->cur, rule.tk_ty)->tk.info = tk.info;

			return CONTINUE;
		} else {
			restore_token(p, tk);

			next_variant(p);

			return CONTINUE;
		}
	case BSYMBOL_NONTERMINAL:
		p->cur = new_nt_node(p->cur, rule.nt_ty);

		return RESTORE;
	}

	b_unreachable()
}

enum b_parser_result b_parser_try_next(struct b_parser *p, struct bsymbol *out)
{
	if (p->root == NULL)
		p->cur = p->root = new_nt_node(NULL, BNT_STMT);

	struct btoken tk;
	enum feed_result res;

	while ((tk = next_token(p)).ty != BTK_NOTOKEN) {
		res = feed_parser(p, tk);

		switch (res) {
		case CONTINUE:
			continue;

		case READY:
			*out = *p->root;
			p->root = NULL;
			// fall through
		case NOMATCH:
		case RESTORE:
			restore_token(p, tk);
			break;
		}

		if (p->root == NULL)
			return BPARSER_READY;

		if (res == NOMATCH)
			return BPARSERE_NOMATCH;
	}

	return BPARSER_CONTINUE;
}
