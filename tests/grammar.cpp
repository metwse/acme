#include "../src/rdesc.hpp"
#include "../src/grammar.hpp"
#include "../src/lex.hpp"

#include <rdesc/rdesc.h>
#include <rdesc/util.h>

#include <cstdio>
#include <cassert>
#include <memory>
#include <sstream>
#include <string>

using std::shared_ptr;
using std::stringstream;
using std::string;


void test_grammar(shared_ptr<Cfg>cfg, const char *input) {
    auto parser = cfg->new_parser();

    parser.start(NT_STMT);

    stringstream ss;
    ss << input;

    Lex lex { ss };
    struct rdesc_node *out = NULL;
    for (auto tk = lex.next(); tk.id != TK_NOTOKEN; tk = lex.next())
        parser.pump(&out, &tk);

    assert(out);

    rdesc_dump_dot(out, tk_printer, nt_names, stdout);

    rdesc_node_destroy(out, tk_destroyer);
}

int main() {
    auto cfg = load_grammar();

    test_grammar(cfg, "lut<2, 1> nand = (0b0111) { _shape: [] };");
}
