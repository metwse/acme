#include "../bgrammar.h"

#include <stdio.h>

#include "common.h"

#include "../grammar.c"


void print_rule(const struct production rule[MAX_BODY_LENGTH])
{
	for (int i = 0; i < MAX_BODY_LENGTH; i++) {
		if (rule[i].end == EOB) {
			if (i == 0)
				putchar('E');

			break;
		}
		if (i)
			putchar(' ');

		if (rule[i].ty == BSYMBOL_TOKEN) {
			const char *tk = btokens[rule[i].tk_ty];

			if (strlen(tk) && tk[0] == '@') {
				printf("%s", tk + 1);
			} else {
				putchar('"');
				printf("%s", tk);
				putchar('"');
			}
		} else {
			putchar('<');
			print_tolower(bnonterminals[rule[i].nt_ty]);
			putchar('>');
		}
	}
}

int main()
{
	for (enum bnt_type head = 0; head < BNONTERMINAL_COUNT; head++) {
		if (head != 0)
			putchar('\n');

		int padding = strlen(bnonterminals[head]) + 5;

		putchar('<');
		print_tolower(bnonterminals[head]);
		printf("> ::= ");

		for (int rule = 0; productions[head][rule][0].end != EOC; rule++) {
			if (rule != 0) {
				putchar('\n');
				for (int i = 0; i < padding; i++)
					putchar(' ');
				printf("| ");
			}

			print_rule(productions[head][rule]);
		}

		printf("\n");
		if (head != BNONTERMINAL_COUNT - 1)
			putchar('\n');
	}

	return 0;
}
