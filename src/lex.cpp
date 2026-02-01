#include "lex.hpp"
#include "grammar.hpp"

#include <rdesc/cfg.h>

#include <cstddef>
#include <cctype>
#include <string>

using std::string;


struct rdesc_cfg_token Lex::next() {
    char input;
    for (input = ' '; isspace(input) && !s.eof(); input = s.get())
        ;

    if (s.eof())
        return { TK_NOTOKEN, nullptr };

    // TODO: Implement more generic way for handling multi-character
    // punctuation
    if (input == '-') {
        char peek = s.get();
        if (peek == '>')
            return { TK_RARROW, nullptr };
        else
            return { TK_NOTOKEN, nullptr };  // sytax error
    }

    s.unget();

    string token_str;

    size_t token_len;
    bool valid_ident = true, valid_num = true;

    int number_base = 10;

    for (token_len = 0;; token_len++) {
        input = s.get();
        if (s.eof())
            return { TK_NOTOKEN, nullptr };

        if (isspace(input))
            break;

        bool punctuation_break = false;
        for (int i = TK_LPAREN; i <= TK_EQ; i++)
            if (input == tk_names[i][0]) {
                if (token_len == 0) {
                    return { i, nullptr }; // punctuation
                } else {
                    // One-character "punctuation" can break an identifier or
                    // num read. Hold the character for the next lexeme.
                    punctuation_break = true;
                    break;
                }
            }

        // transfer break from inner loop to outer for loop.
        if (punctuation_break) {
            s.unget();
            break;
        }

        bool skip_input = false;
        if (token_len == 0 && input == '0') {
            valid_ident = false;

            char number_base_identifier = s.get();

            skip_input = true;
            switch (number_base_identifier) {
            case 'b':
                number_base = 2;
                break;
            case 'o':
                number_base = 8;
                break;
            case 'x':
                number_base = 16;
                break;
            default:
                skip_input = false;
                s.unget();
            }
        } else {
            if (!(isdigit(input) ||
                  (number_base == 16 && (
                    ('a' <= input && input <= 'f') ||
                    ('A' <= input && input <= 'F')
                  ))
               ))
                valid_num = false;

            if (!(isalnum(input) || input == '_'))
                valid_ident = false;

            if (!valid_ident && !valid_num) {
                s.unget();
                break;
            }
        }

        // token_len is the length of raw token. 0b, 0o, and 0x omitted so
        // the length of token_str may be less than token_len.
        if (!skip_input)
            token_str += input;
    }

    // DO NOT REORDER valid_num_int -> valid_ident check order, as valid_ident
    // variable is true for num tokens
    if (token_len && valid_num) {
        auto *seminfo = new NumInfo { number_base, token_str };

        return { TK_NUM, seminfo };
    } else if (token_len && valid_ident) {
        for (int i = TK_LUT; i <= TK_UNIT; i++)
            if (token_str == tk_names[i]) {
                return { i, nullptr }; // keyword
            }

        auto *seminfo = new IdentInfo { Lex::ident_id(token_str) };

        return { TK_IDENT, seminfo };
    }

    return { TK_NOTOKEN, nullptr };
}

size_t Lex::ident_id(const std::string &s) {
    size_t &id = idents[s];

    if (id == 0) {
        id = ++last_ident_id;
    }

    return id;
}
