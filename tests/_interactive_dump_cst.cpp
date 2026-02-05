#include "../include/rdesc.hpp"
#include "../include/grammar.hpp"
#include "../include/lex.hpp"
#include "../src/detail.h"

#include <rdesc/cfg.h>
#include <rdesc/rdesc.h>
#include <rdesc/util.h>

#include <string>

using std::cin;
using std::string;


int main() {
    auto parser = global_cfg()->new_parser();
    global_cfg();

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
