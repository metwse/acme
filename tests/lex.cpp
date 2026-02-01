#include "../src/lex.hpp"
#include "../src/grammar.hpp"

#include <cassert>
#include <sstream>

using std::stringstream;


int main() {
    stringstream ss;
    ss << "lut<2, 1> nand = (0b0111);"
       << "wire a = 1;"
       << "unit<nand> uut1 = (a, b) -> (c);";

    auto tokens = {
        TK_LUT, TK_LANGLE_BRACKET, TK_NUM, TK_COMMA, TK_NUM, TK_RANGLE_BRACKET,
            TK_IDENT, TK_EQ, TK_LPAREN, TK_NUM, TK_RPAREN, TK_SEMI,

        TK_WIRE, TK_IDENT, TK_EQ, TK_NUM, TK_SEMI,

        TK_UNIT, TK_LANGLE_BRACKET, TK_IDENT, TK_RANGLE_BRACKET, TK_IDENT,
            TK_EQ, TK_LPAREN, TK_IDENT, TK_COMMA, TK_IDENT, TK_RPAREN,
            TK_RARROW, TK_LPAREN, TK_IDENT, TK_RPAREN, TK_SEMI,
    };

    Lex lex { ss };

    for (auto token : tokens) {
        enum tk lex_token = (enum tk) lex.next().id;

        assert(lex_token == token);
    }
}
