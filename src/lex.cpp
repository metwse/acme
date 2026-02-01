#include "lex.hpp"
#include "grammar.hpp"

#include <cstdio>
#include <rdesc/cfg.h>

#include <cstddef>
#include <cctype>
#include <string>

using std::string;


struct rdesc_cfg_token Lex::next() {
    struct rdesc_cfg_token out { .id = TK_NOTOKEN, .seminfo = NULL };

    string *token_str = new string;

    size_t token_len;
    bool valid_ident = true, valid_num = true;
    bool possible_num_kind_selection = false;

    bool collect_multichar_punctuation = false;

    char input;
    for (token_len = 0;; token_len++) {

        if (peek) {
            input = peek;
            peek = 0;
        } else {
            input = s.get();
            if (s.eof()) {
                delete token_str;
                return out;
            }
        }

        // TODO: better way for handling RARROW
        if (token_len == 0 && input == '-') {
            collect_multichar_punctuation = true;
            continue;
        } else if (collect_multichar_punctuation) {
            if (input == '>') {
                out.id = TK_RARROW;
                delete token_str;
                return out; // RARROW
            }

            delete token_str;
            return out; // sytax error
        }

        if (possible_num_kind_selection) {
            if (!isdigit(input)) {
                switch (input) {
                case 'b':
                    break; // TODO
                case 'o':
                    break; // TODO
                case 'x':
                    break; // TODO
                default:
                    delete token_str;
                    return out; // sytax error
                }
            }
        }

        if (isspace(input)) {
            if (token_len == 0) {
                token_len--;
                continue;
            } else {
                break;
            }
        }

        for (int i = TK_LPAREN; i <= TK_EQ; i++)
            if (input == tk_names[i][0]) {
                if (token_len == 0) {
                    out.id = i;
                    delete token_str;
                    return out; // end of buffer
                } else {
                    // One-character "punctuation" can break an identifier or
                    // num read. Hold the character for the next lexeme.
                    peek = input;
                    break;
                }
            }

        // transfer break from inner loop to outer for loop.
        if (peek)
            break;

        if (token_len == 0 && input == '0') {
            valid_ident = false;
            possible_num_kind_selection = true;
        } else {
            if (!isdigit(input)) {
                if (possible_num_kind_selection)
                    possible_num_kind_selection = false;
                else
                    valid_num = false;
            }

            if (!(isalnum(input) || input == '_'))
                valid_ident = false;

            if (!valid_ident && !valid_num) {
                peek = input;
                break;
            }
        }

        *token_str += input;
    }

    // DO NOT REORDER valid_num_int -> valid_ident check order, as valid_ident
    // variable is true for num tokens
    if (token_len && valid_num) {
        out.id = TK_NUM;
        out.seminfo = token_str;
    } else if (token_len && valid_ident) {
        for (int i = TK_LUT; i <= TK_UNIT; i++)
            if (*token_str == tk_names[i]) {
                out.id = i;
                delete token_str;
                return out; // keyword
            }

        out.id = TK_IDENT;
        out.seminfo = token_str;
    } else {
        delete token_str;
    }

    return out;
}
