/**
 * @file lex.hpp
 * @brief Lexer operates on input streams
 */

#ifndef LEX_HPP
#define LEX_HPP


#include <rdesc/cfg.h>

#include <cstddef>
#include <map>
#include <istream>
#include <utility>


class Lex {
public:
    Lex(std::istream &s_)
        : s{ s_.rdbuf() } {}

    Lex(const Lex &other) = delete;

    ~Lex() = default;

    struct rdesc_cfg_token next();

private:
    size_t get_ident_id(const std::string &);

    char skip_space();

    struct rdesc_cfg_token lex_num(char c);
    struct rdesc_cfg_token lex_ident_or_keyword(char c);
    struct rdesc_cfg_token lex_punctuation(char c);

    std::istream s;
    std::map<std::string, size_t> idents;
    size_t last_ident_id;
};

class SemInfo {
public:
    virtual ~SemInfo() = default;
};

class NumInfo : public SemInfo {
public:
    NumInfo(int base_, std::string num_)
        : base { base_ }, num { std::move(num_) } {}

    virtual ~NumInfo() = default;

    int base;
    std::string num;
};

class IdentInfo : public SemInfo {
public:
    IdentInfo(size_t id_)
        : id { id_ } {}

    virtual ~IdentInfo() = default;

    size_t id;
};

#endif
