#include "../include/interpreter.hpp"
#include "../include/core.hpp"
#include "../include/lex.hpp"
#include "../src/detail.h"  // IWYU pragma: keep

#include <rdesc/rdesc.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

using std::string;
using std::stringstream;


template<typename E>
void tests_should_fail(const char *input) {
    auto parser = global_cfg()->new_parser();

    stringstream ss;
    ss << input;
    Lex lex { ss };

    Interpreter intr { std::move(parser) };

    try {
        struct rdesc_cfg_token tk;
        while ((tk = lex.next()).id != TK_EOF)
            assert(intr.pump(tk) != RDESC_NOMATCH,
                   "syntax error");

        assert(0, "test should be failed");  // GCOVR_EXCL_LINE
    } catch (E &) {}
}

int main() {
    tests_should_fail<std::length_error>(
        "lut<2, 1> and = (0b111, 0);"
    );

    tests_should_fail<std::invalid_argument>(
        "lut<1, 1> buf = (1);"
        "unit<buf> a = (unknown_wire_1) -> (unknown_wire_2);"
    );

    tests_should_fail<std::invalid_argument>(
        "wire a = 1; wire b = 2;"
        "unit<unknown_lut> a = (a) -> (b);"
    );

    tests_should_fail<std::length_error>(
        "lut<2, 1> and = (8); wire a = 1; wire b = 1;"
        "unit<and> unt = (a) -> (b);"
    );

    tests_should_fail<std::length_error>(
        "lut<2, 1> and = (8); wire a = 1; wire b = 0; wire c = 0; wire d = 0;"
        "unit<and> unt = (a, b) -> (c, d);"
    );
}
