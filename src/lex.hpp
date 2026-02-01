/**
 * @file lex.hpp
 * @brief Lexer operates on input streams
 */

#ifndef LEX_HPP
#define LEX_HPP


#include <rdesc/cfg.h>

#include <istream>


class Lex {
public:
    Lex(std::istream &s_)
        : s{ s_.rdbuf() } {}

    Lex(const Lex &other) = delete;

    ~Lex() = default;

    struct rdesc_cfg_token next();

private:
    std::istream s;
    char peek = 0;
};


#endif
