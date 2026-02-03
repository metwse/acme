#include "../src/detail.h"  // IWYU pragma: keep
#include "../src/core.hpp"
#include "../src/lex.hpp"

#include <rdesc/rdesc.h>

#include <sstream>
#include <string>
#include <utility>

using std::string;
using std::stringstream;


void test_perfect_grammar(const char *input) {
    auto cfg = load_grammar();
    auto parser = cfg->new_parser();

    stringstream ss;

    ss << input;

    Lex lex { ss };

    Interpreter intr { std::move(parser) };

    struct rdesc_cfg_token tk;
    while ((tk = lex.next()).id != TK_EOF)
        assert(intr.pump(tk) != RDESC_NOMATCH,
               "syntax error");

    stringstream intr_dump;

    intr_dump << intr;

    assert(intr_dump.str() == input, "grammar mismatch");
}

int main() {
    test_perfect_grammar(
        /* and gate */
        "lut<2, 1> l1 = (0b1000);\n"
        "\n"
        "wire w2 = 0;\n"
        "wire w3 = 1;\n"
        "wire w4 = 0;\n"
        "\n"
        "unit<l1> u5 = (w2, w3) -> (w4);\n"
    );

    test_perfect_grammar(
        /* 2-bit adder */
        "lut<4, 3> l1 = (0b1110110010000000, 0b1001001101101100, 0b1010101010101010);\n"
        "\n"
        "wire w2 = 0;\n"
        "wire w3 = 1;\n"
        "wire w4 = 0;\n"
        "wire w5 = 1;\n"
        "wire w6 = 0;\n"
        "wire w7 = 1;\n"
        "wire w8 = 0;\n"
        "\n"
        "unit<l1> u9 = (w2, w3, w4, w5) -> (w6, w7, w8);\n"
    );
}
