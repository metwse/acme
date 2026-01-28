/**
 * @file grammar.h
 * @brief HDL grammar definition.
 */

#ifndef GRAMMAR_H
#define GRAMMAR_H


#include <rdesc/cfg.h>

/** @brief add TK_ prefix in `TK` macro */
#define PREFIX_TK(tk) TK_ ## tk
/** @brief add NT_ prefix in `NT` macro */
#define PREFIX_NT(nt) NT_ ## nt

#include <rdesc/bnf_dsl.h>


#define TK_COUNT 20

#define NT_COUNT 13
#define NT_VARIANT_COUNT 5
#define NT_BODY_LENGTH 16


enum tk {
	TK_NOTOKEN,

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

	/* tokentree for table values */
	TK_TABLE_VALUE,
};

enum nt {
	/* statements */
	NT_STMT, NT_LUT, NT_WIRE, NT_UNIT,

	NT_NUM_LS, NT_NUM_LS_REST,

	NT_IDENT_LS, NT_IDENT_LS_REST,

	NT_TABLE, NT_OPTTABLE,
	NT_TABLE_ENTRY, NT_TABLE_ENTRY_LS, NT_TABLE_ENTRY_LS_REST,
};

const char *const tk_names[TK_COUNT] = {
	"",

	"@num", "@ident",

	"(", ")",
	"<", ">",
	"[", "]",
	"{", "}",
	",", ":", ";",
	"=",

	"->",

	"lut", "wire", "unit",

	"@table_value"
};

const char *const tk_names_escaped[TK_COUNT] = {
	"",

	"@num", "@ident",

	"(", ")",
	"<", ">",
	"[", "]",
	"\\{", "\\}",
	",", ":", ";",
	"=",

	"->",

	"lut", "wire", "unit",

	"@table_value"
};

/** @brief non-terminal names (for debugging/printing CST) */
const char *const nt_names[NT_COUNT] = {
	"stmt", "lut", "wire", "unit",

	"num_ls", "num_ls_rest",

	"ident_ls", "ident_ls_rest",

	"table", "opttable",
	"table_entry", "table_entry_ls", "table_entry_ls_rest",
};

const struct rdesc_cfg_symbol
grammar[NT_COUNT][NT_VARIANT_COUNT][NT_BODY_LENGTH] = {
	/* <stmt> ::= */ r(
		TK(SEMI),
	alt	NT(LUT), TK(SEMI),
	alt	NT(WIRE), TK(SEMI),
	alt	NT(UNIT), TK(SEMI),
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
		TK(IDENT), TK(COLON), TK(TABLE_VALUE),
	),
	/* <table_entry_ls> ::= */
		rrr(TABLE_ENTRY_LS, NT(TABLE_ENTRY), TK(COMMA)),
};

#endif
