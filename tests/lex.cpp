#include "../include/lex.hpp"
#include "../include/grammar.hpp"
#include "../src/detail.h"

#include <array>
#include <string>
#include <cstddef>
#include <sstream>

using std::stringstream;
using std::array;
using std::string;


template<size_t size>
void test_grammar(array<enum tk, size> token_ids,
                  const char *input) {
    stringstream ss;
    ss << input;
    Lex lex { ss };

    for (auto token_id : token_ids) {
        auto lex_token = lex.next();

        auto lex_token_id = (enum tk) lex_token.id;
        auto lex_token_seminfo = (SemInfo *) lex_token.seminfo;

        delete lex_token_seminfo;

        assert(lex_token_id == token_id, "token mismatch");
    }
}

template<size_t size>
void test_num(array<int, size> base_,
              array<string, size> num_,
              const char *input) {
    stringstream ss;
    ss << input;
    Lex lex { ss };

    for (size_t i = 0; i < size; i++) {
        auto token = lex.next();

        auto base = base_[i];
        auto num = num_[i];

        auto seminfo = (NumInfo *) token.seminfo;

        assert(token.id == TK_NUM, "token mismatch");
        assert(num == seminfo->num, "num value mismatch");
        assert(base == seminfo->base, "base mismatch");

        delete seminfo;
    }
}

int main() {
    test_grammar(array {
        TK_LUT, TK_LANGLE_BRACKET, TK_NUM, TK_COMMA, TK_NUM, TK_RANGLE_BRACKET,
            TK_IDENT, TK_EQ, TK_LPAREN, TK_NUM, TK_RPAREN, TK_SEMI,

        TK_WIRE, TK_IDENT, TK_EQ, TK_NUM, TK_LCURLY, TK_IDENT,
            TK_COLON, TK_LPAREN, TK_NUM, TK_COMMA, TK_NUM, TK_RPAREN, TK_RCURLY,
            TK_SEMI,

        TK_UNIT, TK_LANGLE_BRACKET, TK_IDENT, TK_RANGLE_BRACKET, TK_IDENT,
            TK_EQ, TK_LPAREN, TK_IDENT, TK_COMMA, TK_IDENT, TK_RPAREN,
            TK_RARROW, TK_LPAREN, TK_IDENT, TK_RPAREN, TK_SEMI,

        TK_EOF,
    }, "lut<2, 1> nand = (0b0111);"
       "wire a = 1 { a: (1, 2) };"
       "unit<nand> /*  */uut1 /**/= (a, b) -> (c);");

    test_grammar(array {
        TK_IDENT, TK_NUM, TK_NUM, TK_NUM, TK_NUM, TK_NUM, TK_NUM,

        TK_EOF,
    }, " a 0xAa 0o0 0b101011 /*   */01 1 0 /* */");

    test_grammar(array {
        TK_LCURLY, TK_IDENT, TK_COLON, TK_NUM, TK_COMMA, TK_IDENT,
            TK_COLON, TK_LBRACKET, TK_LPAREN, TK_NUM, TK_COMMA, TK_NUM,
            TK_RPAREN, TK_COMMA, TK_IDENT, TK_RBRACKET, TK_RCURLY,

        TK_EOF,
    }, " { _pos: 123, _shape: [(1, 2), ident] } ");

    test_grammar(array { TK_IDENT, TK_COLON, TK_EOF, }, "a: ");
    test_grammar(array { TK_IDENT, TK_COLON, TK_IDENT, }, "a: /**/ a");

    // number continued with alhanumeric
    test_grammar(array { TK_NOTOKEN, }, "123a");

    // unknown punctuation
    test_grammar(array { TK_NOTOKEN, }, "?");

    // unknown token just after the identifier
    test_grammar(array { TK_NOTOKEN, }, "a?");

    // malformed arrow
    test_grammar(array { TK_NOTOKEN, }, "-<");

    // malformed num
    test_grammar(array { TK_NOTOKEN, }, "0x");

    // / should followed by *
    test_grammar(array { TK_NOTOKEN, }, "/");

    // unterminated comment
    test_grammar(array { TK_NOTOKEN, }, "/* *");

    test_num(array { 10, 10, 10, 10 },
             { "01", "1", "0", "1" },
             "001 01 0 1  ");

    test_num(array { 2, 16, 8, 16 },
             { "11", "aA", "0", "1" },
             " 0b11 0xaA /* */ 0o0 0x1");
}
