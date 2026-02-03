#include "../src/detail.h"
#include "../src/rdesc.hpp"
#include "../src/grammar.hpp"
#include "../src/lex.hpp"

#include <rdesc/cfg.h>
#include <rdesc/rdesc.h>
#include <rdesc/util.h>

#include <string>

using std::cin;
using std::string;


int main() {
    auto cfg = load_grammar();
    auto parser = cfg->new_parser();

    parser.start(NT_STMT);

    Lex lex { cin };
    struct rdesc_node *out = NULL;
    struct rdesc_cfg_token tk;

    do {
        tk = lex.next();
        assert(parser.pump(&out, &tk) != RDESC_NOMATCH,
               "could not parse grammar");
    } while (!out);

    assert(out, "could not complete CST");

    rdesc_dump_dot(out, tk_printer, nt_names, stdout);

    rdesc_node_destroy(out, tk_destroyer);
}
