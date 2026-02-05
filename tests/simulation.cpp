#include "../include/core.hpp"
#include "../include/lex.hpp"
#include "../src/detail.h"  // IWYU pragma: keep

#include <iostream>
#include <rdesc/rdesc.h>

#include <sstream>
#include <string>
#include <utility>

using std::string;
using std::stringstream;



int main() {
    auto parser = global_cfg()->new_parser();

    stringstream ss;
    ss << "lut<2, 1> and2 = (0b1000);"
        "lut<2, 1> or2 = (0b1110);"
        "lut<2, 1> nand2 = (0b0111);"
        "lut<2, 1> nor2 = (0b0001);"
        "lut<2, 1> xor2 = (0b0110);"

        "wire a1 = 1; wire a2 = 1;"
        "wire b1 = 1; wire b2 = 0;"
        "wire c1 = 0; wire c2 = 1;"
        "wire d1 = 1; wire d2 = 1;"
        "wire e1 = 0; wire e2 = 1;"
        "wire o = 0;"

        "unit<and2> a = (a2, a1) -> (d1);"
        "unit<or2> b = (b2, b1) -> (d2);"

        "unit<nand2> c = (c2, c1) -> (e2);"

        "unit<xor2> d = (d2, d1) -> (e1);"

        "unit<nor2> e = (e2, e1) -> (o);";

    Lex lex { ss };

    Interpreter intr { std::move(parser) };

    struct rdesc_cfg_token tk;
    while ((tk = lex.next()).id != TK_EOF)
        assert(intr.pump(tk) != RDESC_NOMATCH,
               "syntax error");

    Simulation sim { intr };

    sim.set_wire_state(10, 1);
    sim.stabilize();

    std::cout<<intr;
}
