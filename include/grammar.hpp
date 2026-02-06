/**
 * @file grammar.hpp
 * @brief HDL grammar definition.
 */

#ifndef GRAMMAR_HPP
#define GRAMMAR_HPP


#include <rdesc/cfg.h>

#include <memory>

class Cfg;  /* defined in rdesc.hpp */

/** @brief add TK_ prefix in `TK` macro */
#define PREFIX_TK(tk) TK_ ## tk
/** @brief add NT_ prefix in `NT` macro */
#define PREFIX_NT(nt) NT_ ## nt

#include <rdesc/bnf_dsl.h>

/** @brief Total number of tokens. */
#define TK_COUNT 20

/** @brief Total number of non-terminals. */
#define NT_COUNT 19
/** @brief Maximum number of variants in a non-terminal. */
#define NT_VARIANT_COUNT 5
/** @brief Maximum number of production symbols in a rule. */
#define NT_BODY_LENGTH 16

/** @brief Token IDs. */
enum tk {
    TK_NOTOKEN, TK_EOF,

    /* literals */
    TK_NUM, TK_IDENT,

    /* punctuation */
    TK_LPAREN, TK_RPAREN,
    TK_LANGLE_BRACKET, TK_RANGLE_BRACKET,
    TK_LBRACKET, TK_RBRACKET,
    TK_LCURLY, TK_RCURLY,
    TK_COMMA, TK_COLON, TK_SEMI,
    TK_EQ,

    /* multiple-char punctuation */
    TK_RARROW,

    /* keywords and reserved names */
    TK_LUT, TK_WIRE, TK_UNIT,
};

/** @brief Non-terminal IDs. */
enum nt {
    /* statements */
    NT_STMT, NT_LUT, NT_WIRE, NT_UNIT,

    NT_NUM_LS, NT_NUM_LS_REST,

    NT_IDENT_LS, NT_IDENT_LS_REST,

    NT_TABLE, NT_OPTTABLE,
    NT_TABLE_ENTRY, NT_TABLE_ENTRY_LS, NT_TABLE_ENTRY_LS_REST,

    NT_TABLE_VALUE,
    NT_TV_POINT, NT_TV_POINT_LS, NT_TV_POINT_LS_REST,
    NT_TV_POINT_LS_DELIM, NT_TV_PATH,
};

/** @brief Human-readable token names. */
const char *const tk_names[TK_COUNT] = {
    "@notoken", "@eof",

    "@num", "@ident",

    "(", ")",
    "<", ">",
    "[", "]",
    "{", "}",
    ",", ":", ";",
    "=",

    "->",

    "lut", "wire", "unit",
};

/** @brief Token names with symbols escaped for dotlang graph. */
const char *const tk_names_escaped[TK_COUNT] = {
    "@notoken", "@eof",

    "@num", "@ident",

    "(", ")",
    "\\<", "\\>",
    "[", "]",
    "\\{", "\\}",
    ",", ":", ";",
    "=",

    "-\\>",

    "lut", "wire", "unit",
};

/** @brief non-terminal names (for debugging/printing CST) */
const char *const nt_names[NT_COUNT] = {
    "stmt", "lut", "wire", "unit",

    "num_ls", "num_ls_rest",

    "ident_ls", "ident_ls_rest",

    "table", "opttable",
    "table_entry", "table_entry_ls", "table_entry_ls_rest",

    "table_value",
    "tv_point", "tv_point_ls", "tv_point_ls_rest",
    "tv_point_ls_delim", "tv_path",
};

/** @brief Context-free grammar. */
const struct rdesc_cfg_symbol
grammar[NT_COUNT][NT_VARIANT_COUNT][NT_BODY_LENGTH] = {
    /* <stmt> ::= */ r(
        TK(SEMI),
    alt NT(LUT), TK(SEMI),
    alt NT(WIRE), TK(SEMI),
    alt NT(UNIT), TK(SEMI),
    ),

    /* <lut> ::= */ r(
        TK(LUT), TK(LANGLE_BRACKET),
        TK(NUM), TK(COMMA), TK(NUM),
        TK(RANGLE_BRACKET), TK(IDENT), TK(EQ),
        TK(LPAREN), NT(NUM_LS), TK(RPAREN),
        NT(OPTTABLE),
    ),
    /* <wire> ::= */ r(
        TK(WIRE), TK(IDENT), TK(EQ), TK(NUM), NT(OPTTABLE),
    ),
    /* <unit> ::= */ r(
        TK(UNIT), TK(LANGLE_BRACKET), TK(IDENT), TK(RANGLE_BRACKET),
        TK(IDENT), TK(EQ),
        TK(LPAREN), NT(IDENT_LS), TK(RPAREN), TK(RARROW),
        TK(LPAREN), NT(IDENT_LS), TK(RPAREN),
        NT(OPTTABLE),
    ),

    /* <num_ls> ::= */
        rrr(NUM_LS, TK(NUM), TK(COMMA)),

    /* <ident_ls> ::= */
        rrr(IDENT_LS, TK(IDENT), TK(COMMA)),

    /* <table> ::= */ r(
        TK(LCURLY), NT(TABLE_ENTRY_LS), TK(RCURLY),
    ),
    /* <opttable> ::= */
        ropt(NT(TABLE)),
    /* <table_entry> ::= */ r(
        TK(IDENT), TK(COLON), NT(TABLE_VALUE),
    ),
    /* <table_entry_ls> ::= */
        rrr(TABLE_ENTRY_LS, NT(TABLE_ENTRY), TK(COMMA)),

    /* <table_value> ::= */ r(
        TK(NUM),
    alt NT(TV_POINT),
    alt NT(TV_PATH),
    ),

    /* <tv_point> ::= */ r(
        TK(IDENT),
    alt TK(LPAREN), TK(NUM), TK(COMMA), TK(NUM), TK(RPAREN),
    ),
    /* <tv_point_ls> ::= */
        rrr(TV_POINT_LS, NT(TV_POINT), NT(TV_POINT_LS_DELIM)),
    /* <tv_point_ls_delim> ::= */ r(
        TK(COMMA),
    alt TK(SEMI),
    ),
    /* <tv_path> ::= */ r(
        TK(LBRACKET), NT(TV_POINT_LS), TK(RBRACKET),
    )
};


std::shared_ptr<Cfg> global_cfg();

void tk_printer(const struct rdesc_cfg_token *, FILE *out);

void tk_destroyer(struct rdesc_cfg_token *);


#endif
