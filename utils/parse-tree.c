// tests for bparser's internal functions

#include "../bdef.h"
#include "../bgrammar.h"
#include "../bio.h"
#include "../bparser.h"

#include <stdio.h>
#include <string.h>

#include "common.h"

#include "../tests/mock_input_stream.h"


void dump_graph_recursive(struct bsymbol *sym, b_umem parent_id, b_umem *uid)
{
	b_umem this, root_uid;
	if (!parent_id) {
		root_uid = 1;
		uid = &root_uid;
		this = 1;
	} else {
		this = ++(*uid);
		printf("\t%llu -> %llu;\n", parent_id, this);
	}

	if (sym->ty == BSYMBOL_NONTERMINAL) {
		printf("\t%llu [label=\"<", this);
		print_tolower(bnonterminals[sym->nt.ty]);
		printf(">\"];\n");

		for (b_umem i = 0; i < sym->nt.child_count; i++)
			dump_graph_recursive(sym->nt.children[i], this, uid);

		++(*uid);
		if (!sym->nt.child_count) {
			printf("\t%llu [shape=record,label=\"E\"];\n", *uid);
			printf("\t%llu -> %llu;\n", this, *uid);
		}
	} else {
		const char *tk = btokens[sym->tk.ty];
		if (tk[0] == '@') {
			printf("\t%llu [shape=record,label=\"{{", this);
			print_tolower(btoken_names[sym->tk.ty]);
			putchar('|');
			switch (sym->tk.ty) {
			case BTK_POSITIVE_INT:
				printf("%llu", sym->tk.info.positive_int);
				break;
			case BTK_IDENT:
				printf("%s", sym->tk.info.ident);
				break;
			default:
				break;
			}
			printf("}}\"];\n");
		} else {
			printf("\t%llu [shape=box,label=\"\\\"%s\\\"\"];\n",
			       this, tk);
		}
	}
}

void dump_graph(struct bsymbol *sym)
{
	printf("digraph G {\n");
	dump_graph_recursive(sym, 0, NULL);
	printf("}\n");
}

int main()
{
	struct b_parser parser;
	struct bio bio;

	char buf[1000];
	fgets(buf, 999, stdin);

	bio_init(&bio, mock_input_stream,
		 new_mock_input_stream_state(buf, strlen(buf)));

	b_parser_init(&parser);
	b_parser_setinput(&parser, &bio);

	struct bsymbol stmt;
	b_parser_try_next(&parser, &stmt);

	dump_graph(&stmt);
}
