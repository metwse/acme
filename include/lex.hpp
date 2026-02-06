/**
 * @file lex.hpp
 * @brief Lexer operates on input streams
 */

#ifndef LEX_HPP
#define LEX_HPP


#include "grammar.hpp"

#include <cstdint>
#include <rdesc/cfg.h>

#include <cinttypes>
#include <cstddef>
#include <map>
#include <iostream>
#include <string>
#include <utility>
#include <vector>


/** @brief Tokenizer */
class Lex {
public:
    Lex(std::istream &s_)
        : s { s_.rdbuf() } {}

    /* SAFETY: `struct rdesc` cannot shared across lexers. */
    Lex(const Lex &other) = delete;

    ~Lex() = default;

    struct rdesc_cfg_token next();

    const std::string &ident_name(size_t i) const;

private:
    template<typename T>
    friend void operator<<(Lex &lex, T i);

    size_t get_ident_id(const std::string &);

    char skip_space();

    struct rdesc_cfg_token skip_comment();

    struct rdesc_cfg_token lex_num(char c);
    struct rdesc_cfg_token lex_ident_or_keyword(char c);
    struct rdesc_cfg_token lex_punctuation(char c);

    std::iostream s;
    enum tk lookahead = TK_NOTOKEN;

    std::map<std::string, size_t> idents;
    std::vector<std::string> ident_names;
    size_t last_ident_id = 0;
};

/** @brief Semantic information base class. */
class SemInfo {
public:
    virtual ~SemInfo() = default;
};

/** @brief Semantic information for numeric types. */
class NumInfo : public SemInfo {
public:
    NumInfo(int base_, std::string num_)
        : base { base_ }, num { std::move(num_) } {}

    virtual ~NumInfo() = default;

    uintmax_t decimal() const
        { return strtoumax(num.c_str(), NULL, base); }

    int base;
    std::string num;
};

/** @brief Semantic information for identifiers. */
class IdentInfo : public SemInfo {
public:
    IdentInfo(size_t id_)
        : id { id_ } {}

    virtual ~IdentInfo() = default;

    size_t id;
};


template<typename T>
void operator<<(Lex &lex, T i) {
    lex.s << i;
}


#endif
