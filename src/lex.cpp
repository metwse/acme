#include "lex.hpp"
#include "grammar.hpp"
#include "detail.hpp"

#include <rdesc/cfg.h>

#include <cstddef>
#include <cctype>
#include <string>

using std::string;


struct rdesc_cfg_token Lex::next() {
    char c = skip_space();

    if (isspace(c) || s.eof())
        return { TK_NOTOKEN, nullptr };

    if (lookahead != TK_NOTOKEN) {
        auto lookahead_ = lookahead;
        lookahead = TK_NOTOKEN;

        switch (lookahead_) {
        case TK_COLON:
            return lex_table_value(c);
        default:
            break;
        }
    }

    if (isdigit(c))
        return lex_num(c);

    if (isalnum(c) || c == '_')
        return lex_ident_or_keyword(c);

    return lex_punctuation(c);
}

bool is_breaking(char c) {
    for (int i = TK_LPAREN; i <= TK_RARROW; i++)
        if (c == tk_names[i][0])
            return true;

    return isspace(c);
}

char Lex::skip_space() {
    char c;
    for (c = ' ';
         isspace(c) && !s.eof();
         c = s.get())
        ;

    return c;
}

struct rdesc_cfg_token Lex::lex_num(char c) {
    int base = 10;
    string num;

    if (c == '0') {
        switch (s.peek()) {
        case 'x':
            base = 16;
            break;
        case 'o':
            base = 8;
            break;
        case 'b':
            base = 2;
            break;
        default:
            break;
        }

        if (base != 10)
            s.get();

        c = s.get();
    }

    while (
        ((base == 16 &&
            (isdigit(c) || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F'))) ||
         (base == 10 &&
            isdigit(c)) ||
         (base == 8 &&
            ('0' <= c && c <= '7')) ||
         (base == 2 &&
            (c == '0' || c == '1')))
        && !s.eof()
    ) {
        num += c;
        c = s.get();
    }

    if ((s.eof() || is_breaking(c)) && (num.length() || base == 10)) {
        if (num.length() == 0)
            num += '0';

        auto *seminfo = new NumInfo { base, num };

        if (!s.eof())
            s.unget();

        return { TK_NUM, seminfo };
    } else {
        // syntax error, probably number continued with an alphanumeric
        // character
        return { TK_NOTOKEN, nullptr };
    }
}

struct rdesc_cfg_token Lex::lex_punctuation(char c) {
    if (c == '-') {
        char peek = s.get();
        if (peek == '>')
            return { TK_RARROW, nullptr };
        else
            return { TK_NOTOKEN, nullptr };  // syntax error, malformed rarrow
    }

    for (int i = TK_LPAREN; i <= TK_EQ; i++)
        if (c == tk_names[i][0]) {
            lookahead = (enum tk) i;
            return { i, nullptr }; // punctuation
        }


    return { TK_NOTOKEN, nullptr };
}

struct rdesc_cfg_token Lex::lex_ident_or_keyword(char c) {
    string ident;

    while (
        (isalnum(c) || c == '_')
        && !s.eof()
    ) {
        ident += c;
        c = s.get();
    }

    if (s.eof() || is_breaking(c)) {
        if (!s.eof())
            s.unget();

        for (int i = TK_LUT; i <= TK_UNIT; i++)
            if (ident == tk_names[i]) {
                return { i, nullptr }; // keyword
            }

        auto *seminfo = new IdentInfo { Lex::get_ident_id(ident) };

        return { TK_IDENT, seminfo };
    } else {
        // syntax error, invalid token just after the identifier
        return { TK_NOTOKEN, nullptr };
    }
}

struct rdesc_cfg_token Lex::lex_table_value(char c) {
    if (c == ',' || c == '}') {
        // syntax error, no table value
        return { TK_NOTOKEN, nullptr };
    }

    size_t bracket_depth = 0;
    string table_value;

    while (!(bracket_depth == 0 && (c == ',' || c == '}')) && !s.eof()) {
        if (c == '[')
            bracket_depth++;
        else if (c == ']')
            bracket_depth--;

        table_value += c;
        c = s.get();
    }

    if (s.eof()) {
        // syntax error
        return { TK_NOTOKEN, nullptr };
    } else {
        s.unget();

        auto *seminfo = new TableValueInfo { table_value };

        return { TK_TABLE_VALUE, seminfo };
    }
}

size_t Lex::get_ident_id(const string &s) {
    size_t &id = idents[s];

    if (id == 0)
        id = ++last_ident_id;

    return id;
}

const std::string &Lex::ident_name(size_t i) const {
    assert(0 < i && i <= last_ident_id, "identifier id out of range");

    return ident_names[i + 1];
}
