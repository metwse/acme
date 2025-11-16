#include "bdef.h"
#include "bgrammar.h"
#include "bio.h"
#include "blex.h"
#include "bparser.h"

#include <stdbool.h>
#include <stdlib.h>

#include "bmem.h"
#include "grammar.c"


#define get_body(sym) productions[sym->nt.ty][sym->nt.variant]
#define get_current_rule(sym) get_body(sym)[sym->nt.child_count]

#define is_grammar_complete(sym) get_current_rule(sym).end == EOB
#define is_construct_end(sym) get_body(sym)[0].end == EOC


void b_parser_init(struct b_parser *p)
{
	b_stack_init(&p->tokens);

	p->cur = p->root = NULL;

	b_lex_init(&p->lex);
}

void *b_parser_clearinput(struct b_parser *p)
{
	while (p->tokens.len) {
		struct btoken *tk = cast(struct btoken *,
			   b_stack_pop_elem(&p->tokens, sizeof(struct btoken)));

		if (tk->ty == BTK_IDENT)
			free(tk->info.ident);
	}

	if (p->root)
		 b_parser_destroy_tree(p->root);
	p->cur = p->root = NULL;

	b_stack_reset(&p->tokens);

	return b_lex_clearinput(&p->lex);
}

void *b_parser_setinput(struct b_parser *p, struct bio *bio)
{
	return b_lex_setinput(&p->lex, bio);
}

void b_parser_destroy_tree(struct bsymbol *sym)
{
	switch (sym->ty) {
	case BSYMBOL_NONTERMINAL:
		for (b_umem i = sym->nt.child_count; i > 0; i--)
			b_parser_destroy_tree(sym->nt.children[i - 1]);

		free(sym->nt.children);
		break;

	case BSYMBOL_TOKEN:
		if (sym->tk.ty == BTK_IDENT)
			free(sym->tk.info.ident);
		break;
	}

	free(sym);
}

void push_child(struct bsymbol *parent, struct bsymbol *child)
{
	b_assert_logic(parent->ty == BSYMBOL_NONTERMINAL,
		       "token as a parent of another symbol");

	b_assert_boundary(parent->nt.child_count < child_cap_of(parent->nt.ty),
			  "child capacity of parent is exceeded");

	parent->nt.children[parent->nt.child_count++] = child;
}

void restore_token(struct b_parser *p, struct btoken tk)
{
	b_stack_push_elem(&p->tokens, &tk, sizeof(tk));
}

struct btoken next_token(struct b_parser *p)
{
	if (p->tokens.len == 0) {
		struct btoken out;

		b_lex_next(&p->lex, &out);

		return out;
	} else {
		return *cast(struct btoken *,
			     b_stack_pop_elem(&p->tokens, sizeof(struct btoken)));
	}
}

struct bsymbol *new_nt_node(struct bsymbol *parent,
			    enum bnt_type ty)
{
	struct bsymbol *n = malloc(sizeof(struct bsymbol));
	b_assert_mem(n);

	n->parent = parent;
	if (parent)
		push_child(parent, n);

	n->ty = BSYMBOL_NONTERMINAL;

	b_umem child_cap = child_cap_of(ty);
	b_assert_logic(child_cap, "a nonterminal with no child");
	n->nt.ty = ty;
	n->nt.child_count = 0;
	n->nt.children = malloc(sizeof(struct bsymbol *) * child_cap);
	b_assert_mem(n->nt.children);
	n->nt.variant = 0;

	return n;
}

struct bsymbol *new_tk_node(struct bsymbol *parent,
			    enum btk_type ty)
{
	struct bsymbol *n = malloc(sizeof(struct bsymbol));
	b_assert_mem(n);

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
		struct bsymbol *next_cur = NULL, *child;

		for (b_umem i = p->cur->nt.child_count; i > 0; i--) {
			child = p->cur->nt.children[i - 1];

			if (child->ty == BSYMBOL_NONTERMINAL) {
				next_cur = child;

				break;
			} else {
				p->cur->nt.child_count--;

				restore_token(p, child->tk);
				free(child);
			}
		}

		if (next_cur)
			p->cur = next_cur;
		else
			break;
	}

	p->cur->nt.variant++;
	p->cur->nt.child_count = 0;
}

void backtrace(struct b_parser *p)
{
	struct bsymbol *parent = p->cur->parent, *child = NULL;

	while (child != p->cur)
		b_parser_destroy_tree(
			child = parent->nt.children[--parent->nt.child_count]);
	p->cur = parent;

	next_variant(p);

	if (is_construct_end(p->cur) && p->cur->parent)
		backtrace(p);
}

enum feed_result {
	READY,
	CONTINUE,
	RETRY,
	NOMATCH,
} feed_parser(struct b_parser *p, struct btoken tk)
{
	if (is_grammar_complete(p->cur)) {
		p->cur = p->cur->parent;

		if (p->cur == NULL)
			return READY;
		else
			return RETRY;
	}

	if (is_construct_end(p->cur)) {
		restore_token(p, tk);

		if (p->cur->parent) {
			backtrace(p);

			return CONTINUE;
		} else {
			return NOMATCH;
		}
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

		return RETRY;
	}

	b_unreachable(); // GCOV_EXCL_LINE
}

void b_parser_start(struct b_parser *p, enum bnt_type start_symbol)
{
	b_assert_logic(p->root == NULL, "setting start symbol during parsing");

	p->cur = p->root = new_nt_node(NULL, start_symbol);
}

enum b_parser_result b_parser_continue(struct b_parser *p, struct bsymbol **out)
{
	b_assert_logic(p->root,
		       "continuing an incremental parse with no nonterminal");

	struct btoken tk;

	while ((tk = next_token(p)).ty != BTK_NOTOKEN) {
		enum feed_result res;

		do {
			res = feed_parser(p, tk);
		} while (res == RETRY);

		switch (res) {
		case CONTINUE:
			continue;

		case NOMATCH:
			b_parser_destroy_tree(p->root);

			p->root = p->cur = NULL;

			return BPARSERE_NOMATCH;

		case READY:
			*out = p->root;

			p->cur = p->root = NULL;

			return BPARSER_READY;

		case RETRY: b_unreachable(); // GCOV_EXCL_LINE
		}
	}

	return BPARSER_CONTINUE;
}
