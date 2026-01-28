#include <rdesc/cfg.h>
#include <rdesc/util.h>

#include "../src/grammar.h"


int main()
{
	struct rdesc_cfg cfg;

	rdesc_cfg_init(&cfg, NT_COUNT, NT_VARIANT_COUNT, NT_BODY_LENGTH,
		       (struct rdesc_cfg_symbol *) grammar);

	rdesc_dump_bnf(&cfg, tk_names, nt_names, stdout);

	rdesc_cfg_destroy(&cfg);
}
